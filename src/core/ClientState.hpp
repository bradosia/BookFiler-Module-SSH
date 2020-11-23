/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_CLIENT_STATE_H
#define BOOKFILER_MODULE_SSH_CLIENT_STATE_H

// config
#include "config.hpp"

// C++17
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/asio/ip/address.hpp>

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

class ClientState {
public:
  ClientState();
  ~ClientState();
  unsigned short port;
  // int because libssh2 uses int
  int srcPort, remotePort, remoteBoundPort;
  std::string hostName, userName, password, remoteHostName, srcHostName;
};

} // namespace ssh
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_SSH_CLIENT_STATE_H
