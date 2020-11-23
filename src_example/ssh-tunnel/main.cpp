/*
 * @name BookFiler Module - HTTP w/ Curl
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Bookfiler Modules
#include <BookFilerModuleSshLoader.hpp>

int allModulesLoaded();

std::string testName = "HTTP Server Example";
std::shared_ptr<bookfiler::ssh::ModuleInterface> sshModule;

int main() {
  std::cout << testName << " BEGIN" << std::endl;

  bookfiler::ssh::loadModule("modules", std::bind(&allModulesLoaded),
                             sshModule);

  std::cout << testName << " END" << std::endl;
  return 0;
}

int allModulesLoaded() {
  // Setup server
  std::shared_ptr<bookfiler::ssh::Global> sshGlobal = sshModule->newSshGlobal();
  std::shared_ptr<bookfiler::ssh::TunnelServer> sshTunnelServer =
      sshModule->newTunnelServer({{"sourceAddress", "127.0.0.1"},
                                  {"sourcePort", 3306},
                                  {"destAddress", "SERVER"},
                                  {"destPort", 21098},
                                  {"remoteAddress", "127.0.0.1"},
                                  {"remotePort", 3306},
                                  {"userName", "USER"},
                                  {"password", "PASS"}});

  // Start server
  sshTunnelServer->run();

  std::cout << "\n===MAIN THREAD===\nApplication waiting until shut down.";
  sshModule->wait("exit");
  std::cout << "\n===MAIN THREAD===\nApplication Shutting Done\n";

  return 0;
}
