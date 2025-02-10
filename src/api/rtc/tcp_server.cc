#include "api/rtc/tcp_server.h"
#include <memory>
#include "api/rtc/channel.h"
#include "api/rtc/tcp_server_connection.h"
#include "base/logging.h"
#include "base/task/sequenced_task_runner.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "net/socket/tcp_socket_posix.h"

namespace worker {

scoped_refptr<Channel> TCPServer::Delegate::HandleWebSocketRequest(
    TCPServer* server,
    Connection* connection,
    const HttpRequestInfo& request_info) {
  return nullptr;
}

TCPServer::TCPServer(Delegate* delegate, Http1Processor* http_processor)
    : delegate_(delegate),
      http_processor_(http_processor),
      task_runner_(base::SequencedTaskRunner::GetCurrentDefault()) {
  QUEUE_INIT(&tcp_queue_);
}

TCPServer::~TCPServer() = default;

int TCPServer::InitializeAndListen(const net::IPEndPoint& endpoint) {
  auto socket =
      std::make_unique<net::TCPServerSocket>(nullptr, net::NetLogSource());
  int result = socket->Listen(endpoint, 10, {});
  if (result) {
    LOG(ERROR) << "TCP listen failed: " << net::ErrorToString(result);
    return result;
  }
  listen_socket_ = std::move(socket);
  return net::OK;
}

void TCPServer::Run() {
  task_runner_->PostTask(FROM_HERE, base::BindOnce(&TCPServer::DoAccept,
                                                   weak_factory_.GetWeakPtr()));
}

void TCPServer::DoAccept() {
  while (++sync_accept_count_ < 4) {
    int rv = listen_socket_->Accept(
        &accepted_socket_, base::BindOnce(&TCPServer::OnAcceptCompleted,
                                          weak_factory_.GetWeakPtr()));
    if (rv != net::OK)
      return;

    HandleAcceptResult(std::move(accepted_socket_));
  }

  sync_accept_count_ = 0;
  task_runner_->PostTask(FROM_HERE, base::BindOnce(&TCPServer::DoAccept,
                                                   weak_factory_.GetWeakPtr()));
}

void TCPServer::OnAcceptCompleted(int rv) {
  if (rv == net::OK) {
    HandleAcceptResult(std::move(accepted_socket_));
    DoAccept();
  } else {
    LOG(ERROR) << "Accept failed! error=" << rv;
    DoAccept();
  }
}

void TCPServer::HandleAcceptResult(
    std::unique_ptr<net::StreamSocket> accepted_socket) {
  auto connection = new TCPServerConnection(this, std::move(accepted_socket));
  QUEUE_INSERT_TAIL(&tcp_queue_, &connection->connection_queue);
}

void TCPServer::RemoveConnection(TCPServerConnection* connection) {
  DCHECK(task_runner_->RunsTasksInCurrentSequence());
  delete connection;
}

}  // namespace worker