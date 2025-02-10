#include "api/rtc/io_buffer.h"

namespace worker {

scoped_refptr<IOBuffer> IOBuffer::New(const void* p, size_t size, Type type) {
  IOBuffer* o = reinterpret_cast<IOBuffer*>(new char[sizeof(IOBuffer) + size]);
  o->ref_count_ = 0;
  o->size_ = size;
  o->type_ = type;
  memcpy(o->data(), p, size);

  return scoped_refptr<IOBuffer>(o);
}

scoped_refptr<IOBuffer> IOBuffer::New(int size, Type type) {
  IOBuffer* o = reinterpret_cast<IOBuffer*>(new char[sizeof(IOBuffer) + size]);
  o->ref_count_ = 0;
  o->size_ = size;
  o->type_ = type;
  return scoped_refptr<IOBuffer>(o);
}

scoped_refptr<IOBuffer> IOBuffer::NewWebsocketFrame(std::string_view data,
                                                    bool text) {
  size_t hdr_size = 2;
  const size_t len = data.size();

  if (len < 126) {
    // 2 bytes frame header
  } else if (len < (1 << 16)) {
    hdr_size += 2;
  } else if (static_cast<uint64_t>(len) < (1ull << 63)) {
    hdr_size += 8;
  } else {
    return nullptr;
  }

  const size_t frame_size = len + hdr_size;

  auto buffer = IOBuffer::New(frame_size, IOBuffer::WEBSOCKET_FRAME);
  uint8_t* ptr = buffer->data<uint8_t>();
  *ptr++ = (0x80 | (text ? 1 : 2));  // fin=1, rsv=0
  if (len < 126) {
    *ptr++ = (unsigned char)len;  // no mask
  } else if (len < (1 << 16)) {
    *ptr++ = 126;  // no mask
    *ptr++ = (len >> 8) & 0xff;
    *ptr++ = len & 0xff;
  } else {
    *ptr++ = 127;  // no mask
    uint64_t nplayload = len;
    for (int i = 0; i < 8; i++) {
      ptr[i] = (unsigned char)((nplayload >> ((7 - i) * 8)) & 0xff);
    }
    ptr += 8;
  }
  memcpy(ptr, data.data(), len);
  return buffer;
}

}  // namespace worker
