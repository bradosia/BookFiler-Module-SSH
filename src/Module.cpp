/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

/* rapidjson v1.1 (2016-8-25)
 * Developed by Tencent
 * License: MITs
 */
#include <rapidjson/prettywriter.h>

// Local Project
#include "Module.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

ModuleExport::ModuleExport(){

};

ModuleExport::~ModuleExport(){};

int ModuleExport::init() {
  std::cout << moduleName << ": init()" << std::endl;
  return 0;
}

int ModuleExport::registerSettings(
    std::shared_ptr<rapidjson::Document> moduleRequest,
    std::shared_ptr<std::unordered_map<
        std::string, std::function<void(std::shared_ptr<rapidjson::Document>)>>>
        moduleCallbackMap) {
  moduleRequest->SetObject();
  moduleRequest->AddMember("HTTP_accounts", "HTTP_accountsCB",
                           moduleRequest->GetAllocator());
  moduleCallbackMap->insert(
      {"HTTP_accountsCB",
       std::bind(&ModuleExport::setAccounts, this, std::placeholders::_1)});
  moduleRequest->AddMember("HTTP_settings", "HTTP_settingsCB",
                           moduleRequest->GetAllocator());
  moduleCallbackMap->insert(
      {"HTTP_settingsCB",
       std::bind(&ModuleExport::setSettings, this, std::placeholders::_1)});
  return 0;
}

int ModuleExport::setAccounts(std::shared_ptr<rapidjson::Value> jsonDoc) {
  accountsDoc = jsonDoc;
#if MODULE_EXPORT_SET_ACCOUNTS_DEBUG
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  jsonDoc->Accept(writer);
  std::cout << moduleCode << "::ModuleExport::setAccounts:\n"
            << buffer.GetString() << std::endl;
#endif
  return 0;
}

int ModuleExport::setSettings(std::shared_ptr<rapidjson::Value> jsonDoc) {
  settingsDoc = jsonDoc;
#if MODULE_EXPORT_SET_SETTINGS_DEBUG
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  jsonDoc->Accept(writer);
  std::cout << moduleCode << "::ModuleExport::setSettings:\n"
            << buffer.GetString() << std::endl;
#endif
  return 0;
}

std::shared_ptr<bookfiler::port::ForwardingServer>
ModuleExport::newPortForwarding(
    std::map<std::string, bookfiler::port::newForwardingServerType> map_) {
  std::shared_ptr<bookfiler::port::ForwardingServerImpl> serverPtr =
      std::make_shared<bookfiler::port::ForwardingServerImpl>(map_);
  serverPtr->setSettingsDoc(settingsDoc);
  return std::dynamic_pointer_cast<bookfiler::port::ForwardingServer>(
      serverPtr);
}

std::shared_ptr<TunnelServer>
ModuleExport::newTunnelServer(std::map<std::string, newTunnelServerType> map_) {
  std::shared_ptr<TunnelServerImpl> serverPtr =
      std::make_shared<TunnelServerImpl>(map_);
  serverPtr->setSettingsDoc(settingsDoc);
  return std::dynamic_pointer_cast<TunnelServer>(serverPtr);
}

std::shared_ptr<bookfiler::ssh::Client> ModuleExport::newSshClient(
    std::map<std::string, bookfiler::ssh::newSshClientType> map_) {
  std::shared_ptr<bookfiler::ssh::ClientImpl> clientPtr =
      std::make_shared<bookfiler::ssh::ClientImpl>(map_);
  clientPtr->setSettingsDoc(settingsDoc);
  return std::dynamic_pointer_cast<bookfiler::ssh::Client>(clientPtr);
}

std::shared_ptr<bookfiler::ssh::Global> ModuleExport::newSshGlobal() {
  std::shared_ptr<bookfiler::ssh::GlobalImpl> globalPtr =
      std::make_shared<bookfiler::ssh::GlobalImpl>();
  return std::dynamic_pointer_cast<bookfiler::ssh::Global>(globalPtr);
}

void ModuleExport::wait(const std::string handle_) {
  std::unique_lock<std::mutex> mutexLock(globalMutex);
  conditionVariableMap.emplace(std::piecewise_construct,
                               std::forward_as_tuple(handle_), // args for key
                               std::forward_as_tuple());
  auto it = conditionVariableMap.find(handle_);
  if (it != conditionVariableMap.end()) {
    conditionVariableMap.at(handle_).wait(mutexLock);
  }
}

void ModuleExport::notify(const std::string handle_) {
  auto it = conditionVariableMap.find(handle_);
  if (it != conditionVariableMap.end()) {
    conditionVariableMap.at(handle_).notify_one();
  }
}

} // namespace ssh
} // namespace bookfiler
