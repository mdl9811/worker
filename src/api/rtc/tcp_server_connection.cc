#include "api/rtc/tcp_server_connection.h"
#include <cstdint>
#include "api/rtc/channel.h"
#include "api/rtc/http1_processor.h"
#include "api/rtc/io_buffer.h"
#include "api/rtc/tcp_server.h"
#include "base/base64.h"
#include "base/hash/sha1.h"
#include "base/posix/eintr_wrapper.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/socket/stream_socket.h"
#include "net/socket/tcp_client_socket.h"
#include "net/websockets/websocket_handshake_constants.h"

namespace {
constexpr uint32_t kInitialBufferSize = 4 * 1024;
constexpr uint32_t kMaxHttpPostBodySize = 4 * 1024;
constexpr base::TimeDelta kHandshakeTimeout = base::Seconds(2);
constexpr base::TimeDelta kLiveCheckInterval = base::Seconds(2);
constexpr base::TimeDelta kLiveTimeout = base::Seconds(3);

constexpr uint8_t kHttpGetSignagure[] = {'G', 'E', 'T', ' '};
constexpr uint8_t kHttpPostSignagure[] = {'P', 'O', 'S', 'T'};

const net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation(
        "TCP server channel",
        "semantics {\n"
        "  sender: \"Worker TCP server channel\"\n"
        "  description: \"Worker TCP server channel\"\n"
        "  trigger: \"N/A\"\n"
        "  data: \"No user data.\"\n"
        "  destination: OTHER\n"
        "  destination_other: \"N/A\"\n"
        "}\n"
        "policy {\n"
        "  cookies_allowed: NO\n"
        "  setting: \"This request cannot be disabled.\"\n"
        "  policy_exception_justification: \"none\"\n"
        "}");

enum OP_CODE {
  WS_CONTINUATION_FRAME = 0x0u,
  WS_TEXT_FRAME = 0x1u,
  WS_BINARY_FRAME = 0x2u,
  WS_CONNECTION_CLOSE = 0x8u,
  WS_PING = 0x9u,
  WS_PONG = 0xau
};

// Input character types.
enum header_parse_inputs {
  INPUT_LWS,
  INPUT_CR,
  INPUT_LF,
  INPUT_COLON,
  INPUT_DEFAULT,
  MAX_INPUTS,
};

// Parser states.
enum header_parse_states {
  ST_METHOD,     // Receiving the method
  ST_URL,        // Receiving the URL
  ST_PROTO,      // Receiving the protocol
  ST_HEADER,     // Starting a Request Header
  ST_NAME,       // Receiving a request header name
  ST_SEPARATOR,  // Receiving the separator between header name and value
  ST_VALUE,      // Receiving a request header value
  ST_DONE,       // Parsing is complete and successful
  ST_ERR,        // Parsing encountered invalid syntax.
  MAX_STATES
};

// State transition table
const int kParserState[MAX_STATES][MAX_INPUTS] = {
    /* METHOD    */ {ST_URL, ST_ERR, ST_ERR, ST_ERR, ST_METHOD},
    /* URL       */ {ST_PROTO, ST_ERR, ST_ERR, ST_URL, ST_URL},
    /* PROTOCOL  */ {ST_ERR, ST_HEADER, ST_NAME, ST_ERR, ST_PROTO},
    /* HEADER    */ {ST_ERR, ST_ERR, ST_NAME, ST_ERR, ST_ERR},
    /* NAME      */ {ST_SEPARATOR, ST_DONE, ST_ERR, ST_VALUE, ST_NAME},
    /* SEPARATOR */ {ST_SEPARATOR, ST_ERR, ST_ERR, ST_VALUE, ST_ERR},
    /* VALUE     */ {ST_VALUE, ST_HEADER, ST_NAME, ST_VALUE, ST_VALUE},
    /* DONE      */ {ST_DONE, ST_DONE, ST_DONE, ST_DONE, ST_DONE},
    /* ERR       */ {ST_ERR, ST_ERR, ST_ERR, ST_ERR, ST_ERR}};

// Convert an input character to the parser's input token.
int CharToInput(char ch) {
  switch (ch) {
    case ' ':
    case '\t':
      return INPUT_LWS;
    case '\r':
      return INPUT_CR;
    case '\n':
      return INPUT_LF;
    case ':':
      return INPUT_COLON;
  }
  return INPUT_DEFAULT;
}

bool ParseHeaders(const char* data,
                  size_t data_len,
                  worker::HttpRequestInfo* info,
                  size_t* ppos) {
  size_t& pos = *ppos;
  int state = ST_METHOD;
  std::string buffer;
  std::string header_name;
  std::string header_value;
  while (pos < data_len) {
    char ch = data[pos++];
    const int input = CharToInput(ch);
    int next_state = kParserState[state][input];

    bool transition = (next_state != state);
    decltype(info->headers)::iterator it;
    if (transition) {
      // Do any actions based on state transitions.
      switch (state) {
        case ST_METHOD:
          if (buffer[0] == 'P')
            info->method = worker::HttpRequestInfo::POST;
          else
            info->method = worker::HttpRequestInfo::GET;
          buffer.clear();
          break;
        case ST_URL: {
          size_t qpos = buffer.find('?');
          if (qpos != buffer.npos) {
            info->path = buffer.substr(1, qpos - 1);
            info->query = buffer.substr(qpos + 1);
            buffer.clear();
          } else {
            info->path = std::move(buffer);
          }
        } break;
        case ST_PROTO:
          if (buffer != "HTTP/1.1" && buffer != "HTTP/1.0") {
            next_state = ST_ERR;
          }
          buffer.clear();
          break;
        case ST_NAME:
          header_name = base::ToLowerASCII(buffer);
          buffer.clear();
          break;
        case ST_VALUE:
          base::TrimWhitespaceASCII(buffer, base::TRIM_LEADING, &header_value);
          it = info->headers.find(header_name);
          // See the second paragraph ("A sender MUST NOT generate multiple
          // header fields...") of tools.ietf.org/html/rfc7230#section-3.2.2.
          if (it == info->headers.end()) {
            info->headers[header_name] = header_value;
          } else {
            it->second.append(",");
            it->second.append(header_value);
          }
          buffer.clear();
          break;
        case ST_SEPARATOR:
          break;
      }
      state = next_state;
    } else {
      // Do any actions based on current state
      switch (state) {
        case ST_METHOD:
        case ST_URL:
        case ST_PROTO:
        case ST_VALUE:
        case ST_NAME:
          buffer.append(&ch, 1);
          break;
        case ST_DONE:
          // We got CR to get this far, also need the LF
          return (input == INPUT_LF);
        case ST_ERR:
          return false;
      }
    }
  }
  // No more characters, but we haven't finished parsing yet. Signal this to
  // the caller by setting |pos| to zero.
  pos = 0;
  return true;
}
}  // namespace

namespace worker {

TCPServerConnection::TCPServerConnection(
    TCPServer* server,
    std::unique_ptr<net::StreamSocket> socket)
    : server_(server), socket_(std::move(socket)) {
  input_buffer_ = base::MakeRefCounted<net::GrowableIOBuffer>();
  input_buffer_->SetCapacity(kInitialBufferSize);

  handshake_timeout_ = std::make_unique<base::OneShotTimer>();
  handshake_timeout_->Start(
      FROM_HERE, kHandshakeTimeout,
      base::BindOnce(&TCPServerConnection::OnHandshakeTimeout,
                     base::Unretained(this)));
  server_->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&TCPServerConnection::StartIdentify,
                                base::Unretained(this)));
  message_writer_ = &TCPServerConnection::DoWrite;
}

TCPServerConnection::~TCPServerConnection() = default;

void TCPServerConnection::HandleError(int rv) {
  if (rv < 0)
    LOG(INFO) << "TCPServerConnection::HandleError:" << rv;
  server_->RemoveConnection(this);
}

void TCPServerConnection::OnHandshakeTimeout() {
  LOG(INFO) << "HandshakeTimeout";
  HandleError(net::ERR_TIMED_OUT);
}

void TCPServerConnection::OnLiveCheckTimer() {
  if (pending_request_)
    return;
  if (output_buffer_)
    return;
  if ((base::Time::Now() - last_recv_time_) > kLiveTimeout) {
    // LOG(INFO) << "Timeout! kLiveTimeout = " << kLiveTimeout;
    HandleError(net::OK);
  }
}

void TCPServerConnection::OnHandshakeDone(int rv) {
  if (rv != net::OK) {
    LOG(ERROR) << "Handshake failed:" << rv;
    HandleError(rv);
    return;
  }
  handshake_timeout_.reset();
  last_recv_time_ = base::Time::Now();
  live_check_timer_ = std::make_unique<base::RepeatingTimer>();
  live_check_timer_->Start(
      FROM_HERE, kLiveCheckInterval,
      base::BindRepeating(&TCPServerConnection::OnLiveCheckTimer,
                          base::Unretained(this)));
  StartReading();
}

void TCPServerConnection::Send(scoped_refptr<IOBuffer> buffer) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  output_queue_.emplace_back(std::move(buffer));
  if (!output_buffer_)
    (this->*message_writer_)();
}

void TCPServerConnection::Close() {
  LOG(INFO) << "TCPServerConnection::Close";
  server_->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&TCPServerConnection::HandleError,
                                weak_factory_.GetWeakPtr(), net::ERR_ABORTED));
}

void TCPServerConnection::DidConsume(int bytes) {
  DCHECK_GE(bytes, 0);

  uint8_t* data = input_buffer_->everything().data();
  int data_size = input_buffer_->offset();
  memmove(data, data + bytes, data_size - bytes);
  input_buffer_->set_offset(data_size - bytes);
}

void TCPServerConnection::HandleReadResult(int result) {
  if (result <= 0) {
    LOG(INFO) << "HandleReadResult:" << result;
    HandleError(result);
    return;
  }

  last_recv_time_ = base::Time::Now();
  input_buffer_->set_offset(input_buffer_->offset() + result);
  int data_size = input_buffer_->offset();
  if (data_size < 4) {
    uint8_t* d = input_buffer_->everything().data();
    int result = 1;
    switch (d[0]) {
      case 'G':
        result = memcmp(d, kHttpGetSignagure, data_size);
        break;
      case 'P':
        result = memcmp(d, kHttpPostSignagure, data_size);
        break;
      default:
        result = 1;
        break;
    }

    if (result == 0) {
      StartReading();
      return;
    }

    LOG(INFO) << "Unknown protocol, drop connection!";
    HandleError(net::ERR_INVALID_ARGUMENT);
    return;
  }

  // At least 4 bytes has been read
  uint8_t* d = input_buffer_->everything().data();
  switch (d[0]) {
    case 'G':
      if (d[1] == 'E' && d[2] == 'T' && d[3] == ' ') {
        input_buffer_->set_offset(0);
        HandleHttpReadResult(data_size);
        return;
      }
      break;
    case 'P':
      if (d[1] == 'O' && d[2] == 'S' && d[3] == 'T') {
        input_buffer_->set_offset(0);
        HandleHttpReadResult(data_size);
        return;
      }
      break;
    default:
      break;
  }
  LOG(INFO) << "Unknown protocol, drop connection!";
  HandleError(net::ERR_INVALID_ARGUMENT);
}

void TCPServerConnection::DoWebSocketWrite() {
  if (!output_buffer_) {
    if (!output_queue_.size())
      return;
    std::vector<scoped_refptr<IOBuffer>> output_queue;
    output_queue.swap(output_queue_);
    int total_size = 0;

    for (const auto& i : output_queue) {
      if (i->type() == IOBuffer::WEBSOCKET_FRAME) {
        total_size += i->size();
      } else {
        size_t hdr_size = 2;
        const int size = i->size();
        if (size < 126) {
          // 2 bytes frame header
        } else if (size < (1 << 16)) {
          hdr_size += 2;
        } else {
          hdr_size += 8;
        }

        total_size += (size + hdr_size);
      }
    }

    auto combined_buffer =
        base::MakeRefCounted<net::IOBufferWithSize>(total_size);
    char* data = combined_buffer->data();
    for (const auto& i : output_queue) {
      if (i->type() == IOBuffer::WEBSOCKET_FRAME) {
        memcpy(data, i->data(), i->size());
        data += i->size();
      } else {
        size_t hdr_size = 2;
        const int size = i->size();
        if (size < 126) {
          // 2 bytes frame header
        } else if (size < (1 << 16)) {
          hdr_size += 2;
        } else {
          hdr_size += 8;
        }

        uint8_t* ptr = reinterpret_cast<uint8_t*>(data);
        *ptr++ = (0x80 | WS_BINARY_FRAME);  // fin=1, rsv=0

        if (size < 126) {
          *ptr++ = size;  // no mask
        } else if (size < (1 << 16)) {
          *ptr++ = 126;  // no mask
          *ptr++ = (size >> 8) & 0xff;
          *ptr++ = size & 0xff;
        } else {
          *ptr++ = 127;  // no mask
          const uint64_t nplayload = size;
          for (int i = 0; i < 8; i++) {
            ptr[i] = (nplayload >> ((7 - i) * 8)) & 0xff;
          }
          ptr += 8;
        }

        memcpy(ptr, i->data(), i->size());
        data += (size + hdr_size);
      }
    }

    output_buffer_ = base::MakeRefCounted<net::DrainableIOBuffer>(
        combined_buffer.get(), total_size);
  }

  int result = socket_->Write(
      output_buffer_.get(), output_buffer_->BytesRemaining(),
      base::BindOnce(&TCPServerConnection::HandleWebSocketWriteResult,
                     weak_factory_.GetWeakPtr()),
      kTrafficAnnotation);
  if (result != net::ERR_IO_PENDING)
    HandleWebSocketWriteResult(result);
}

void TCPServerConnection::HandleWebSocketWriteResult(int result) {
  if (result <= 0) {
    Close();
    return;
  }

  output_buffer_->DidConsume(result);
  if (output_buffer_->BytesRemaining() <= 0) {
    output_buffer_ = nullptr;
    if (output_queue_.size())
      DoWebSocketWrite();
    return;
  }

  DoWebSocketWrite();
}

void TCPServerConnection::OnWebsocketPing() {
  if (!output_buffer_) {
    if ((base::Time::Now() - last_recv_time_) < base::Seconds(10))
      return;

    auto buffer = IOBuffer::New(2, IOBuffer::WEBSOCKET_FRAME);
    uint8_t* ptr = buffer->data<uint8_t>();
    ptr[0] = (0x80 | WS_PING);  // WS_PING
    ptr[1] = 0;
    output_queue_.emplace_back(std::move(buffer));
    DoWebSocketWrite();
  }
}

void TCPServerConnection::HandleWebSocketReadResult(int result) {
  if (result <= 0) {
    LOG(INFO) << "HandleWebSocketReadResult:" << result;
    HandleError(result);
    return;
  }
  DCHECK_GE(read_req_, 2);

  last_recv_time_ = base::Time::Now();
  input_buffer_->set_offset(input_buffer_->offset() + result);

  uint8_t* ptr = input_buffer_->everything().data();
  int data_size = input_buffer_->offset();
  int bytes_processed = 0;
  bool got_ping = false;

  while (data_size >= read_req_) {
    const uint8_t flag = ptr[0];
    const uint8_t imask = (ptr[1] >> 7) & 1;
    if (!imask) {  // !mask || !fin
      shutdown_read_ = true;
      LOG(INFO) << "Invalid websocket frame";
      HandleError(net::ERR_CONNECTION_CLOSED);
      return;
    }

    // hdr + mask
    if (data_size < 6) {
      read_req_ = 6;
      break;
    }

    const uint8_t opcode = flag & 0xfu;
    const uint8_t npayload = ptr[1] & 0x7fu;
    if ((((opcode >> 3) & 1)) && (npayload > 127)) {
      shutdown_read_ = true;
      LOG(INFO) << "Invalid websocket frame";
      HandleError(net::ERR_CONNECTION_CLOSED);
      return;
    }

    size_t nplayload = 0;
    size_t frame_size = 6;
    ptr += 2;

    if (npayload == 126) {
      if (data_size < (6 + 2)) {
        read_req_ = 8;
        break;
      }
      nplayload = (static_cast<size_t>(ptr[0]) << 8) | (ptr[1]);
      ptr += 2;
      frame_size += 2;
    } else if (npayload == 127) {
      if (data_size < (6 + 8)) {
        read_req_ = 14;
        break;
      }
      uint64_t payload = 0;
      for (int i = 0; i < 8; i++)
        payload |= (((uint64_t)ptr[7 - i]) << (i * 8));

      if (payload > INT_MAX) {  // only allow int max size
        shutdown_read_ = true;
        break;
      }
      ptr += 8;

      nplayload = payload;
      frame_size += 8;
    } else {
      nplayload = npayload;
    }

    frame_size += nplayload;
    if (static_cast<int>(frame_size) > data_size) {
      read_req_ = frame_size;
      if (input_buffer_->capacity() < frame_size)
        input_buffer_->SetCapacity((frame_size + 0xFF) & ~0xFF);
      break;
    }

    if (nplayload != 0) {
      uint8_t* mask = ptr;
      ptr += 4;

      // inplace unmask payload
      uint8_t* p = ptr;
      for (size_t i = 0; i < nplayload; i++) {
        *p ^= mask[i & 3];
        p++;
      }
    }

    switch (opcode) {
      default:
      case WS_CONNECTION_CLOSE:
        LOG(INFO) << "WS_CONNECTION_CLOSE";
        HandleError(net::ERR_CONNECTION_CLOSED);
        return;
      case WS_PING:
        got_ping = 1;
        break;
      case WS_PONG:
        break;
      case WS_CONTINUATION_FRAME:
      case WS_TEXT_FRAME:
      case WS_BINARY_FRAME:
        if (flag & 0x80) {
          if (ws_pending_frame_.size()) {
            ws_pending_frame_.append(reinterpret_cast<char*>(ptr), nplayload);
            channel_->OnMessage(
                reinterpret_cast<const uint8_t*>(ws_pending_frame_.data()),
                ws_pending_frame_.size());
            ws_pending_frame_.clear();
          } else {
            channel_->OnMessage(ptr, nplayload);
          }
        } else {
          ws_pending_frame_.append(reinterpret_cast<char*>(ptr), nplayload);
        }
        break;
    }

    ptr += nplayload;
    data_size -= frame_size;
    bytes_processed += frame_size;
    read_req_ = 2;  // continue to read next header
  }

  DidConsume(bytes_processed);

  if (got_ping) {
    auto buffer = IOBuffer::New(2, IOBuffer::WEBSOCKET_FRAME);
    uint8_t* ptr = buffer->data<uint8_t>();
    ptr[0] = (0x80 | WS_PONG);  // PONG
    ptr[1] = 0;
    output_queue_.emplace_back(std::move(buffer));
    if (!output_buffer_)
      DoWebSocketWrite();
  }

  StartWebSocketReading();
}

void TCPServerConnection::StartWebSocketReading() {
  DCHECK(input_buffer_.get());
  DCHECK_GT(input_buffer_->RemainingCapacity(), 0);

  int result = socket_->Read(
      input_buffer_.get(), input_buffer_->RemainingCapacity(),
      base::BindOnce(&TCPServerConnection::HandleWebSocketReadResult,
                     weak_factory_.GetWeakPtr()));
  if (result == net::ERR_IO_PENDING) {
    sync_read_count_ = 0;
    return;
  }

  if (++sync_read_count_ > 16) {
    sync_read_count_ = 0;
    server_->task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&TCPServerConnection::HandleWebSocketReadResult,
                       weak_factory_.GetWeakPtr(), result));
  } else {
    HandleWebSocketReadResult(result);
  }
}

void TCPServerConnection::DeliverConnected() {
  StartWebSocketReading();
}

void TCPServerConnection::HandleHttpResponse(
    const HttpResponseInfo& response_info) {
  pending_request_ = false;
  Send(response_info.Serialize());
}

void TCPServerConnection::HandleHttpReadResult(int result) {
  if (result <= 0) {
    HandleError(result);
    return;
  }

  last_recv_time_ = base::Time::Now();
  input_buffer_->set_offset(input_buffer_->offset() + result);

  uint8_t* data = input_buffer_->everything().data();
  int data_size = input_buffer_->offset();
  int bytes_processed = 0;

  while (data_size > 0) {
    HttpRequestInfo request(secure_);
    size_t frame_size = 0;

    if (!ParseHeaders(reinterpret_cast<char*>(data), data_size, &request,
                      &frame_size)) {
      LOG(INFO) << "ParseHeaders failed";
      HandleError(net::ERR_CONNECTION_CLOSED);
      return;
    } else if (!frame_size) {
      if (input_buffer_->RemainingCapacity() < 0x100) {
        if (input_buffer_->capacity() > 0x800) {
          LOG(INFO) << "input_buffer_->capacity() > 0x800";
          HandleError(net::ERR_CONNECTION_CLOSED);
          return;
        }
        input_buffer_->SetCapacity((input_buffer_->capacity() + 0xFF) & ~0xFF);
      }
      break;
    }

    if (request.HasHeaderValue("connection", "upgrade") &&
        request.HasHeaderValue("upgrade", "websocket")) {
      std::string version = request.GetHeaderValue("sec-websocket-version");
      if (version != "8" && version != "13") {
        LOG(INFO) << "Unsupported websocket version:" << version;
        HandleError(net::ERR_CONNECTION_CLOSED);
        return;
      }
      std::string key = request.GetHeaderValue("sec-websocket-key");
      if (key.empty()) {
        LOG(INFO) << "Invalid websocket key";
        HandleError(net::ERR_CONNECTION_CLOSED);
        return;
      }

      if (output_queue_.size() || output_buffer_.get()) {
        HandleError(net::ERR_CONNECTION_CLOSED);
        return;
      }

      auto channel = server_->HandleWebSocketRequest(this, request);
      if (channel) {
        channel_ = std::move(channel);
        DidConsume(bytes_processed + frame_size);
        message_writer_ = &TCPServerConnection::DoWebSocketWrite;

        std::string accept_hash = base::Base64Encode(
            base::SHA1HashString(key + net::websockets::kWebSocketGuid));
        std::string response = base::StringPrintf(
            "HTTP/1.1 101 WebSocket Protocol Handshake\r\n"
            "Upgrade: WebSocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n"
            "\r\n",
            accept_hash.c_str());
        auto frame = IOBuffer::New(response, IOBuffer::WEBSOCKET_FRAME);
        output_queue_.emplace_back(std::move(frame));
        server_->task_runner()->PostTask(
            FROM_HERE, base::BindOnce(&TCPServerConnection::DoWebSocketWrite,
                                      weak_factory_.GetWeakPtr()));
        read_req_ = 2;
        shutdown_read_ = false;
        shutdown_write_ = false;

        ws_ping_timer_ = std::make_unique<base::RepeatingTimer>();
        ws_ping_timer_->Start(FROM_HERE, base::Seconds(5), this,
                              &TCPServerConnection::OnWebsocketPing);
        server_->task_runner()->PostTask(
            FROM_HERE, base::BindOnce(&TCPServerConnection::DeliverConnected,
                                      weak_factory_.GetWeakPtr()));
      } else {
        LOG(INFO) << "HandleWebSocketRequest failed";
        HandleError(net::ERR_CONNECTION_CLOSED);
      }

      return;
    } else if (request.method == HttpRequestInfo::POST) {
      std::string str = request.GetHeaderValue("content-length");
      int length = 0;
      if (str.size() > 0 && !base::StringToInt(str, &length)) {
        LOG(INFO) << "Invalid content-length:" << str;
        HandleError(net::ERR_CONNECTION_CLOSED);
        return;
      }

      if (length < 0 || length > kMaxHttpPostBodySize) {
        LOG(INFO) << "Invalid content-length:" << length;
        HandleError(net::ERR_CONNECTION_CLOSED);
        return;
      }

      if (data_size < frame_size + length)
        break;

      request.body =
          std::string(reinterpret_cast<char*>(data) + frame_size, length);
      frame_size += length;
    }

    Http1Processor* processor = server_->http_processor();
    if (!processor) {
      LOG(INFO) << "No http processor";
      HandleError(net::ERR_CONNECTION_CLOSED);
      return;
    }

    pending_request_ = true;
    if (!processor->Process(
            std::move(request),
            base::BindOnce(&TCPServerConnection::HandleHttpResponse,
                           weak_factory_.GetWeakPtr()))) {
      HandleError(net::ERR_CONNECTION_CLOSED);
      return;
    }

    data += frame_size;
    data_size -= frame_size;
    bytes_processed += frame_size;
  }

  DidConsume(bytes_processed);
  StartHttpReading();
}

void TCPServerConnection::StartHttpReading() {
  DCHECK(input_buffer_.get());
  DCHECK_GT(input_buffer_->RemainingCapacity(), 0);

  int result =
      socket_->Read(input_buffer_.get(), input_buffer_->RemainingCapacity(),
                    base::BindOnce(&TCPServerConnection::HandleHttpReadResult,
                                   base::Unretained(this)));
  if (result == net::ERR_IO_PENDING) {
    sync_read_count_ = 0;
    return;
  }

  if (++sync_read_count_ > 16) {
    sync_read_count_ = 0;
    server_->task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&TCPServerConnection::HandleHttpReadResult,
                                  weak_factory_.GetWeakPtr(), result));
  } else {
    HandleHttpReadResult(result);
  }
}

void TCPServerConnection::StartReading() {
  DCHECK(input_buffer_.get());
  DCHECK_GT(input_buffer_->RemainingCapacity(), 0);

  int result =
      socket_->Read(input_buffer_.get(), input_buffer_->RemainingCapacity(),
                    base::BindOnce(&TCPServerConnection::HandleReadResult,
                                   weak_factory_.GetWeakPtr()));
  if (result == net::ERR_IO_PENDING) {
    sync_read_count_ = 0;
    return;
  }

  if (++sync_read_count_ > 16) {
    sync_read_count_ = 0;
    server_->task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&TCPServerConnection::HandleReadResult,
                                  weak_factory_.GetWeakPtr(), result));
  } else {
    HandleReadResult(result);
  }
}

void TCPServerConnection::StartIdentify() {
  DCHECK_GE(input_buffer_->capacity(), 4);
  int fd = reinterpret_cast<net::TCPClientSocket*>(socket_.get())
               ->SocketDescriptorForTesting();
  do {
    char c;
    int rv = HANDLE_EINTR(recv(fd, &c, 1, MSG_PEEK));
    if (rv == -1 && errno == EWOULDBLOCK) {
      server_->task_runner()->PostTask(
          FROM_HERE, base::BindOnce(&TCPServerConnection::StartIdentify,
                                    base::Unretained(this)));
      return;
    }
    if ((rv == 0) || (rv == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
      break;
    if (rv < 0) {
      LOG(ERROR) << "Identify faild!";
      return;
    }
    if (c == 'G' || c == 'P' /* || c == 'H' || c == 'C' */) {
      handshake_timeout_.reset();
      OnHandshakeDone(net::OK);
      return;
    } else if (c == 22) {  // TLS handshake
      // TODO
    }
  } while (false);
  OnHandshakeDone(net::ERR_FAILED);
}

void TCPServerConnection::HandleWriteResult(int result) {
  if (result <= 0) {
    Close();
    return;
  }

  DCHECK_GE(output_buffer_->BytesRemaining(), result);

  output_buffer_->DidConsume(result);
  if (output_buffer_->BytesRemaining() <= 0) {
    output_buffer_ = nullptr;
    if (output_queue_.size())
      DoWrite();
    return;
  }

  DoWrite();
}

void TCPServerConnection::DoWrite() {
  if (!output_buffer_) {
    if (!output_queue_.size())
      return;
    int total_size = 0;

    for (const auto& i : output_queue_)
      total_size += i->size();

    auto combined_buffer =
        base::MakeRefCounted<net::IOBufferWithSize>(total_size);
    char* data = combined_buffer->data();

    for (const auto& i : output_queue_) {
      memcpy(data, i->data(), i->size());
      data += i->size();
    }
    output_queue_.clear();

    output_buffer_ = base::MakeRefCounted<net::DrainableIOBuffer>(
        combined_buffer.get(), total_size);
  }

  int result =
      socket_->Write(output_buffer_.get(), output_buffer_->BytesRemaining(),
                     base::BindOnce(&TCPServerConnection::HandleWriteResult,
                                    weak_factory_.GetWeakPtr()),
                     kTrafficAnnotation);
  if (result != net::ERR_IO_PENDING)
    HandleWriteResult(result);
}
}  // namespace worker