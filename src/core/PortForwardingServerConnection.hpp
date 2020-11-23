/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_PORT_FORWARDING_SERVER_CONNECTION_H
#define BOOKFILER_MODULE_PORT_FORWARDING_SERVER_CONNECTION_H

// config
#include "config.hpp"

// C++17
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>

/* rapidjson v1.1 (2016-8-25)
 * Developed by Tencent
 * License: MITs
 */
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/reader.h> // rapidjson::ParseResult
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/strand.hpp>

// Local Project
#include "PortForwardingServerState.hpp"
#include "PortForwardingUtil.hpp"

/*
 * bookfiler - port
 * Port fowarding utilities. Initially this utility was made to forward mysql
 * traffic through a secure ssh connection.
 */
namespace bookfiler {
namespace port {

class ServerState;

// Handles an HTTP server connection
class ConnectionImpl {
private:
  boost::asio::ip::tcp::socket srcSocket, destSocket;
  std::shared_ptr<ServerState> serverState;
  unsigned int connId;

  static const std::size_t read_buffer_length = 1500;
  char src_read_buffer[read_buffer_length],
      dest_read_buffer[read_buffer_length];
  std::size_t write_buffer_length = read_buffer_length;
  typedef std::pair<void *, size_t> write_buffer_type;
  std::queue<write_buffer_type> src_write_queue, dest_write_queue;

public:
  // Take ownership of the socket
  ConnectionImpl(boost::asio::ip::tcp::socket, boost::asio::ip::tcp::socket,
             std::shared_ptr<ServerState>);
  ~ConnectionImpl();

  // Start the asynchronous operation
  int run(boost::asio::yield_context yieldContext);
  int run2(boost::asio::yield_context yieldContext);
  void src_write_handler(boost::system::error_code, std::size_t);
  void dest_write_handler(boost::system::error_code, std::size_t);
  int setId(unsigned int connId_);
};

} // namespace port
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_MYSQL_HTTP_H
