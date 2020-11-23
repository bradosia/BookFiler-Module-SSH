/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "TunnelServerListener.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

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
  int rc = 0;
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
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logStatus("::Listener::run", "acceptor.async_accept");
#endif
    boost::asio::ip::tcp::socket srcSocket(ioContext);
    acceptor.async_accept(srcSocket, yieldContext[ec]);
    if (ec) {
      logStatus("::Listener::run", "acceptor.async_accept", ec);
      continue;
    }

    connectionIdIncrement++;
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logStatus("::Listener::run", "std::shared_ptr<ConnectionImpl>");
#endif
    std::shared_ptr<ConnectionImpl> conn;
    {
      const std::lock_guard<std::mutex> lock(globalMutex);
      serverState->connectionList.emplace_back(
          std::make_shared<ConnectionImpl>(std::move(srcSocket), serverState));
      conn = serverState->connectionList.back();
    }
    conn->setId(connectionIdIncrement);

// Create a fresh connection from ssh server to the remote server
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logStatus("::Listener::run", "make_shared<ClientImpl>");
#endif
    std::map<std::string, newSshClientType> map_(
        {{"srcHostName", serverState->sourceAddress.to_string()},
         {"srcPort", serverState->sourcePort},
         {"hostName", serverState->destAddressStr},
         {"port", serverState->destPortInt},
         {"remoteHostName", serverState->remoteAddressStr},
         {"remotePort", serverState->remotePortInt},
         {"userName", serverState->userNameStr},
         {"password", serverState->passwordStr}});
    std::shared_ptr<ClientImpl> sshClient = std::make_shared<ClientImpl>(map_);
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logStatus("::Listener::run", "make_shared<ClientImpl>");
#endif
    // blocks
    rc = sshClient->connect();
    if (rc < 0) {
      logStatus("::Listener::run", "sshClient->connect ERROR");
      continue;
    }
    // blocks
    rc = sshClient->tunnel();
    if (rc < 0) {
      logStatus("::Listener::run", "sshClient->tunnel ERROR");
      continue;
    }
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logStatus("::Listener::run", "conn->setSshClient(sshClient)");
#endif
    conn->setSshClient(sshClient);
    logStatus("::Listener::run", "boost::asio::spawn Connection::runPull");
    boost::asio::spawn(
        acceptor.get_executor(),
        std::bind(&ConnectionImpl::runPull, conn, std::placeholders::_1));
    logStatus("::Listener::run", "boost::asio::spawn Connection::runPush");
    boost::asio::spawn(
        acceptor.get_executor(),
        std::bind(&ConnectionImpl::runPush, conn, std::placeholders::_1));
  }
  return 0;
}

} // namespace ssh
} // namespace bookfiler
