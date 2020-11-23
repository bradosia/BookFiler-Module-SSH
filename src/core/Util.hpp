/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_UTIL_H
#define BOOKFILER_MODULE_SSH_UTIL_H

// config
#include "config.hpp"

// C++17
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

/* libssh2 1.9.0
 * License: BSD
 */
#include <libssh2.h>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/algorithm/string.hpp>

// Local Project
#include "../bookfiler/Util.hpp"
#include <BookFiler-Module-ssh/Interface.hpp>

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

class GlobalImpl : public Global {
public:
  GlobalImpl();
  ~GlobalImpl();
};

void logStatus(std::string functionStr, std::string msg);
void logStatus(std::string functionStr, std::string msg,
               boost::system::error_code ec);
void logConnectionStatus(unsigned int, std::string functionStr,
                         std::string msg);
void logConnectionStatus(unsigned int, std::string functionStr, std::string msg,
                         boost::system::error_code ec);
std::string readFile(std::string path);

} // namespace ssh
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_SSH_UTIL_H
