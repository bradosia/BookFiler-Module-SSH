/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_TUNNEL_SERVER_STATE_H
#define BOOKFILER_MODULE_SSH_TUNNEL_SERVER_STATE_H

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

class ConnectionImpl;

class ServerState {
public:
  ServerState();
  ~ServerState();
  unsigned short sourcePort, destPort;
  int threadsNum, sourcePortInt, destPortInt, remotePortInt;
  boost::asio::ip::address sourceAddress, destAddress;
  std::string sourceAddressStr, destAddressStr, remoteAddressStr, userNameStr,
      passwordStr;
  std::vector<std::shared_ptr<ConnectionImpl>> connectionList;
};

} // namespace ssh
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_PORT_FORWARDING_SERVER_STATE_Hs
