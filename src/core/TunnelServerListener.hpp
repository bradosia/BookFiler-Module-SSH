/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_TUNNEL_SERVER_LISTENER_H
#define BOOKFILER_MODULE_SSH_TUNNEL_SERVER_LISTENER_H

// config
#include "config.hpp"

// C++17
//#include <filesystem>
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/config.hpp>

// Local Project
#include "TunnelServerConnection.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

// Accepts incoming connections and launches the sessions
class ListenerImpl {
private:
  boost::asio::io_context &ioContext;
  boost::asio::ip::tcp::endpoint srcEndpoint, destEndpoint;
  std::shared_ptr<ServerState> serverState;
  unsigned int connectionIdIncrement;

public:
  ListenerImpl(boost::asio::io_context &, boost::asio::ip::tcp::endpoint,
               boost::asio::ip::tcp::endpoint, std::shared_ptr<ServerState>);

  // Start accepting incoming connections
  int run(boost::asio::yield_context yieldContext);
};

} // namespace port
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_PORT_FORWARDING_SERVER_LISTENER_H
