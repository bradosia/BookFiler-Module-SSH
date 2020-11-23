/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "PortForwardingServerConnection.hpp"

/*
 * bookfiler - port
 * Port fowarding utilities. Initially this utility was made to forward mysql
 * traffic through a secure ssh connection.
 */
namespace bookfiler {
namespace port {

ConnectionImpl::ConnectionImpl(boost::asio::ip::tcp::socket srcSocket_,
                       boost::asio::ip::tcp::socket destSocket_,
                       std::shared_ptr<ServerState> serverState_)
    : srcSocket(std::move(srcSocket_)), destSocket(std::move(destSocket_)),
      serverState(serverState_) {}
ConnectionImpl::~ConnectionImpl() {
  srcSocket.close();
  destSocket.close();
}

int ConnectionImpl::run(boost::asio::yield_context yieldContext) {
  logConnectionStatus(connId, "::ConnectionImpl::run", "START");
  bool runFlag = true;
  boost::system::error_code ec;

  // Write a bit
  logConnectionStatus(connId, "::ConnectionImpl::run",
                  "boost::asio::async_write nothing");
  boost::array<char, 512> buf = {{0}};
  boost::asio::async_write(srcSocket, boost::asio::buffer(buf),
                           yieldContext[ec]);
  if (ec) {
    logConnectionStatus(connId, "::ConnectionImpl::run", "boost::asio::async_write",
                    ec);
    return -1;
  }

  while (runFlag) {
    // Read
    logConnectionStatus(connId, "::ConnectionImpl::run", "srcSocket.async_read_some");
    std::size_t bytesTrasferred = srcSocket.async_read_some(
        boost::asio::buffer(src_read_buffer, read_buffer_length),
        yieldContext[ec]);
    if (ec) {
      logConnectionStatus(connId, "::ConnectionImpl::run",
                      "srcSocket.async_read_some", ec);
      return -1;
    }
    // Setup
    void *buffer_data = boost::asio::asio_handler_allocate(
        bytesTrasferred,
        [this](boost::system::error_code ec, std::size_t length) {
          dest_write_handler(ec, length);
        });
    std::stringstream ss;
    ss << "[ allocated buffer 0x" << buffer_data << "[" << bytesTrasferred
       << "]" << std::endl;
    logConnectionStatus(connId, "::ConnectionImpl::run", ss.str());
    std::memcpy(buffer_data, src_read_buffer, bytesTrasferred);
    write_buffer_type buffer(buffer_data, bytesTrasferred);
    dest_write_queue.push(buffer);
    // Write
    logConnectionStatus(connId, "::ConnectionImpl::run",
                    "boost::asio::async_write destSocket");
    std::size_t length = boost::asio::async_write(
        destSocket, boost::asio::buffer(buffer_data, bytesTrasferred),
        yieldContext[ec]);
    if (ec) {
      logConnectionStatus(connId, "::ConnectionImpl::run", "boost::asio::async_write",
                      ec);
      return -1;
    }
    dest_write_handler(ec, length);
  }

  // At this point the connection is closed gracefully
  return 0;
}

int ConnectionImpl::run2(boost::asio::yield_context yieldContext) {
  logConnectionStatus(connId, "::ConnectionImpl::run2", "START");
  bool runFlag = true;
  boost::system::error_code ec;

  // Write a bit
  logConnectionStatus(connId, "::ConnectionImpl::run2",
                  "boost::asio::async_write nothing");
  boost::array<char, 512> buf = {{0}};
  boost::asio::async_write(destSocket, boost::asio::buffer(buf),
                           yieldContext[ec]);
  if (ec) {
    logConnectionStatus(connId, "::ConnectionImpl::run2", "boost::asio::async_write",
                    ec);
    return -1;
  }

  while (runFlag) {
    // Read
    std::size_t bytesTrasferred = destSocket.async_read_some(
        boost::asio::buffer(dest_read_buffer, read_buffer_length),
        yieldContext[ec]);
    if (ec) {
      logConnectionStatus(connId, "::ConnectionImpl::run2",
                      "destSocket.async_read_some", ec);
      return -1;
    }
    // Setup
    void *buffer_data = boost::asio::asio_handler_allocate(
        bytesTrasferred,
        [this](boost::system::error_code ec, std::size_t length) {
          src_write_handler(ec, length);
        });
    std::stringstream ss;
    ss << "[ allocated buffer 0x" << buffer_data << "[" << bytesTrasferred
       << "]" << std::endl;
    logConnectionStatus(connId, "::ConnectionImpl::run", ss.str());
    std::memcpy(buffer_data, dest_read_buffer, bytesTrasferred);
    write_buffer_type buffer(buffer_data, bytesTrasferred);
    src_write_queue.push(buffer);
    // Write
    std::size_t length = boost::asio::async_write(
        srcSocket, boost::asio::buffer(buffer_data, bytesTrasferred),
        yieldContext[ec]);
    if (ec) {
      logConnectionStatus(connId, "::ConnectionImpl::run2",
                      "boost::asio::async_write", ec);
      return -1;
    }
    src_write_handler(ec, length);
  }

  // At this point the connection is closed gracefully
  return 0;
}

void ConnectionImpl::src_write_handler(boost::system::error_code ec, std::size_t) {
  write_buffer_type buffer = src_write_queue.front();
  src_write_queue.pop();
  std::stringstream ss;
  ss << "] deallocating buffer 0x" << buffer.first << "[" << buffer.second
     << "]" << std::endl;
  logConnectionStatus(connId, "::ConnectionImpl::src_write_handler", ss.str());
  boost::asio::asio_handler_deallocate(buffer.first, buffer.second);
  if (ec) {
    logConnectionStatus(connId, "::ConnectionImpl::src_write_handler", " ", ec);
    srcSocket.close();
    destSocket.close();
    return;
  }
}

void ConnectionImpl::dest_write_handler(boost::system::error_code ec, std::size_t) {
  write_buffer_type buffer = dest_write_queue.front();
  dest_write_queue.pop();
  std::stringstream ss;
  ss << "] deallocating buffer 0x" << buffer.first << "[" << buffer.second
     << "]" << std::endl;
  logConnectionStatus(connId, "::ConnectionImpl::dest_write_handler", ss.str());
  boost::asio::asio_handler_deallocate(buffer.first, buffer.second);
  if (ec) {
    logConnectionStatus(connId, "::ConnectionImpl::dest_write_handler", " ", ec);
    srcSocket.close();
    destSocket.close();
    return;
  }
}

int ConnectionImpl::setId(unsigned int connId_) {
  connId = connId_;
  return 0;
}

} // namespace port
} // namespace bookfiler
