/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_EXPORT_H
#define BOOKFILER_MODULE_SSH_EXPORT_H

// config
#include "core/config.hpp"

// c++17
#include <condition_variable>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/config.hpp> // for BOOST_SYMBOL_EXPORT

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

// Local Project
#include "core/Client.hpp"
#include "core/PortForwardingServer.hpp"
#include "core/TunnelServer.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

/*
 * This widget uses the MVC design pattern.
 * This is the controller, the view is a QT widget, and the model is the API
 * storage
 */
class ModuleExport : public ModuleInterface {
private:
  std::shared_ptr<rapidjson::Value> settingsDoc;
  std::shared_ptr<rapidjson::Value> accountsDoc;
  std::map<std::string, std::condition_variable> conditionVariableMap;

public:
  ModuleExport();
  ~ModuleExport();

  /* module typical methods
   * init, registerSettings
   */
  int init();
  /* registerSettings
   * @brief Registers a map of callbacks for handling data from the main
   * application settings file.
   */
  int registerSettings(
      std::shared_ptr<rapidjson::Document>,
      std::shared_ptr<std::unordered_map<
          std::string,
          std::function<void(std::shared_ptr<rapidjson::Document>)>>>);

  // convenience methods for examples
  void wait(const std::string handle_);
  void notify(const std::string handle_);

  // module specific methods
  int setAccounts(std::shared_ptr<rapidjson::Value>);
  int setSettings(std::shared_ptr<rapidjson::Value>);
  std::shared_ptr<bookfiler::port::ForwardingServer> newPortForwarding(
      std::map<std::string, bookfiler::port::newForwardingServerType>);
  std::shared_ptr<TunnelServer>
      newTunnelServer(std::map<std::string, newTunnelServerType>);
  std::shared_ptr<bookfiler::ssh::Client>
      newSshClient(std::map<std::string, bookfiler::ssh::newSshClientType>);
  std::shared_ptr<bookfiler::ssh::Global> newSshGlobal();
};

// Exporting `my_namespace::module` variable with alias name `module`
// (Has the same effect as `BOOST_DLL_ALIAS(my_namespace::module, module)`)
extern "C" BOOST_SYMBOL_EXPORT ModuleExport bookfilerSshModule;
ModuleExport bookfilerSshModule;

} // namespace ssh
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_MYSQL_EXPORT_H
