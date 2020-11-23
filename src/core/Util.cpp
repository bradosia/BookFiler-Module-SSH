/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "Util.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

GlobalImpl::GlobalImpl() {
  int rc = libssh2_init(0);
  if (rc != 0) {
    fprintf(stderr, "libssh2 initialization failed (%d)\n", rc);
  }
};
GlobalImpl::~GlobalImpl() { libssh2_exit(); };

void logStatus(std::string functionStr, std::string msg) {
  ::bookfiler::logStatus(moduleCode, functionStr, msg);
}

void logStatus(std::string functionStr, std::string msg,
               boost::system::error_code ec) {
  ::bookfiler::logStatus(moduleCode, functionStr, msg, ec);
}

void logConnectionStatus(unsigned int socketId, std::string functionStr,
                         std::string msg) {
  ::bookfiler::logConnectionStatus(moduleCode, socketId, functionStr, msg);
}

void logConnectionStatus(unsigned int socketId, std::string functionStr,
                         std::string msg, boost::system::error_code ec) {
  ::bookfiler::logConnectionStatus(moduleCode, socketId, functionStr, msg, ec);
}

std::string readFile(std::string path) { return ::bookfiler::readFile(path); }

} // namespace ssh
} // namespace bookfiler
