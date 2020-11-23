/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "PortForwardingUtil.hpp"

/*
 * bookfiler - port
 * Port fowarding utilities. Initially this utility was made to forward mysql
 * traffic through a secure ssh connection.
 */
namespace bookfiler {
namespace port {

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

} // namespace port
} // namespace bookfiler
