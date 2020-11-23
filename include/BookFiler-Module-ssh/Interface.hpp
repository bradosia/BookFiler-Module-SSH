/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_INTERFACE_H
#define BOOKFILER_MODULE_SSH_INTERFACE_H

// c++17
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */

/* rapidjson v1.1 (2016-8-25)
 * Developed by Tencent
 * License: MITs
 */
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

/*
 * bookfiler - port
 * Port fowarding utilities. Initially this utility was made to forward mysql
 * traffic through a secure ssh connection.
 */
namespace bookfiler {
namespace port {
using newForwardingServerType = std::variant<int, double, std::string>;

class ForwardingServer {
public:
  virtual int run() = 0;
};
} // namespace port
} // namespace bookfiler

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

class Global {};

using newTunnelServerType = std::variant<int, double, std::string>;

class TunnelServer {
public:
  virtual int run() = 0;
};

using newSshClientType = std::variant<int, double, std::string>;

class Client {
public:
  virtual int connect() = 0;
  virtual int tunnel() = 0;
  virtual int command(std::string const &) = 0;
  virtual int send(std::string bufferStr, size_t len) = 0;
};

class ModuleInterface {
public:
  /* module typical methods
   * init, registerSettings
   */
  virtual int init() = 0;
  /* registerSettings
   * @brief Registers a map of callbacks for handling data from the main
   * application settings file.
   */
  virtual int registerSettings(
      std::shared_ptr<rapidjson::Document>,
      std::shared_ptr<std::unordered_map<
          std::string,
          std::function<void(std::shared_ptr<rapidjson::Document>)>>>) = 0;

  // convenience functions for examples
  virtual void wait(const std::string handle_) = 0;
  virtual void notify(const std::string handle_) = 0;

  // module specific methods
  virtual std::shared_ptr<bookfiler::port::ForwardingServer> newPortForwarding(
      std::map<std::string, bookfiler::port::newForwardingServerType>) = 0;
  virtual std::shared_ptr<TunnelServer>
      newTunnelServer(std::map<std::string, newTunnelServerType>) = 0;
  virtual std::shared_ptr<bookfiler::ssh::Client>
      newSshClient(std::map<std::string, bookfiler::ssh::newSshClientType>) = 0;
  virtual std::shared_ptr<bookfiler::ssh::Global> newSshGlobal() = 0;
};
} // namespace ssh
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_SSH_INTERFACE_H
