/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "PortForwardingServerListener.hpp"

/*
 * bookfiler - port
 * Port fowarding utilities. Initially this utility was made to forward mysql
 * traffic through a secure ssh connection.
 */
namespace bookfiler {
namespace port {

ListenerImpl::ListenerImpl(boost::asio::io_context &ioContext_,
                           boost::asio::ip::tcp::endpoint srcEndpoint_,
                           boost::asio::ip::tcp::endpoint destEndpoint_,
                           std::shared_ptr<ServerState> serverState_)
    : ioContext(ioContext_), srcEndpoint(srcEndpoint_),
      destEndpoint(destEndpoint_), serverState(serverState_) {
    connectionIdIncrement = 0;
}

int ListenerImpl::run(boost::asio::yield_context yieldContext) {
  logStatus("::Listener::run", "START");
  boost::system::error_code ec;

  // Open the acceptor
  boost::asio::ip::tcp::acceptor acceptor(ioContext);
  acceptor.open(srcEndpoint.protocol(), ec);
  if (ec) {
    logStatus("::Listener::run", "acceptor.open", ec);
    return -1;
  }

  // Allow address reuse
  acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec) {
    logStatus("::Listener::run", "acceptor.set_option", ec);
    return -1;
  }

  // Bind to the server address
  acceptor.bind(srcEndpoint, ec);
  if (ec) {
    logStatus("::Listener::run", "acceptor.bind", ec);
    return -1;
  }

  // Start listening for connections
  acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    logStatus("::Listener::run", "acceptor.listen", ec);
    return -1;
  }

  for (;;) {
    logStatus("::Listener::run", "destSocket.async_connect");
    boost::asio::ip::tcp::socket destSocket(ioContext);
    destSocket.async_connect(destEndpoint, yieldContext[ec]);
    if (ec) {
      logStatus("::Listener::run", "destSocket.async_connect", ec);
      continue;
    }

    logStatus("::Listener::run", "acceptor.async_accept");
    boost::asio::ip::tcp::socket srcSocket(ioContext);
    acceptor.async_accept(srcSocket, yieldContext[ec]);
    if (ec) {
      logStatus("::Listener::run", "acceptor.async_accept", ec);
      continue;
    }

    connectionIdIncrement++;

    std::shared_ptr<ConnectionImpl> conn;
    {
      const std::lock_guard<std::mutex> lock(globalMutex);
      serverState->connectionList.emplace_back(std::make_shared<ConnectionImpl>(
          std::move(srcSocket), std::move(destSocket), serverState));
      conn = serverState->connectionList.back();
    }
    conn->setId(connectionIdIncrement);
    logStatus("::Listener::run", "boost::asio::spawn Connection::run");
    boost::asio::spawn(
        acceptor.get_executor(),
        std::bind(&ConnectionImpl::run, conn, std::placeholders::_1));
    logStatus("::Listener::run", "boost::asio::spawn Connection::run2");
    boost::asio::spawn(
        acceptor.get_executor(),
        std::bind(&ConnectionImpl::run2, conn, std::placeholders::_1));
  }
  return 0;
}

} // namespace port
} // namespace bookfiler
