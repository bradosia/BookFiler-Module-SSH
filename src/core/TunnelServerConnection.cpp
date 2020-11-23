/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "TunnelServerConnection.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

ConnectionImpl::ConnectionImpl(boost::asio::ip::tcp::socket srcSocket_,
                               std::shared_ptr<ServerState> serverState_)
    : srcSocket(std::move(srcSocket_)), serverState(serverState_) {}
ConnectionImpl::~ConnectionImpl() {
  logConnectionStatus(connId, "::ConnectionImpl::~ConnectionImpl",
                      "CONNECTION CLOSED");
  srcSocket.close();
}

int ConnectionImpl::runPull(boost::asio::yield_context yieldContext) {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logConnectionStatus(connId, "::ConnectionImpl::runPull", "START");
#endif
  bool runFlag = true;
  int rc = 0;
  boost::system::error_code ec;

  while (runFlag) {
    // Read from the remote socket
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logConnectionStatus(connId, "::ConnectionImpl::runPull", "sshClient->read");
#endif
    size_t len;
    std::string bufferStr;
    sshClient->read(bufferStr, len);
    if (len == 0) {
#if BOOKFILER_SSH_TUNNEL_DEBUG
      logConnectionStatus(connId, "::ConnectionImpl::runPull",
                          "sshClient->getRead() empty");
#endif
      continue;
    }

// Write
#if BOOKFILER_SSH_TUNNEL_DEBUG
    std::stringstream ss;
    ss << "boost::asio::async_write srcSocket:"
       << "\nbytes[" << len << "]: " << bufferStr << std::endl;
    logConnectionStatus(connId, "::ConnectionImpl::runPull", ss.str());
#endif
    std::size_t length = boost::asio::async_write(
        srcSocket, boost::asio::buffer(bufferStr.data(), len),
        yieldContext[ec]);
    if (ec) {
      logConnectionStatus(connId, "::ConnectionImpl::runPull",
                          "boost::asio::async_write", ec);
      return -1;
    }
  }

  // At this point the connection is closed gracefully
  logConnectionStatus(connId, "::ConnectionImpl::runPull", "END");
  return 0;
}

int ConnectionImpl::runPush(boost::asio::yield_context yieldContext) {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logConnectionStatus(connId, "::ConnectionImpl::runPush", "START");
#endif
  bool runFlag = true;
  int rc = 0;
  boost::system::error_code ec;

  while (runFlag) {
    // Read from the local end
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logConnectionStatus(connId, "::ConnectionImpl::runPush",
                        "srcSocket.async_read_some");
#endif
    std::size_t bytesTrasferred = srcSocket.async_read_some(
        boost::asio::buffer(src_read_buffer, read_buffer_length),
        yieldContext[ec]);
#if BOOKFILER_SSH_TUNNEL_DEBUG
    std::stringstream ss;
    ss << "srcSocket.async_read_some: "
       << "\nbytes[" << bytesTrasferred
       << "]: " << std::string(src_read_buffer, bytesTrasferred) << std::endl;
#endif
    if (ec) {
      logConnectionStatus(connId, "::ConnectionImpl::runPush",
                          "srcSocket.async_read_some", ec);
      return -1;
    }
// Write
#if BOOKFILER_SSH_TUNNEL_DEBUG
    logConnectionStatus(connId, "::ConnectionImpl::runPush", "sshClient->send");
#endif
    rc = sshClient->send(std::string(src_read_buffer, bytesTrasferred),
                         bytesTrasferred);
    if (rc < 0) {
      logConnectionStatus(connId, "::ConnectionImpl::runPush",
                          "sshClient->send ERROR");
      return -1;
    }
  }

  // At this point the connection is closed gracefully
  logConnectionStatus(connId, "::ConnectionImpl::runPush", "END");
  return 0;
}

int ConnectionImpl::setId(unsigned int connId_) {
  connId = connId_;
  return 0;
}

int ConnectionImpl::setSshClient(std::shared_ptr<ClientImpl> sshClient_) {
  sshClient = sshClient_;
  return 0;
}

} // namespace ssh
} // namespace bookfiler
