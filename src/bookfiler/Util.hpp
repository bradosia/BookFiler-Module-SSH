/*
 * @name BookFiler Utilities
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief Utilities for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_UTIL_H
#define BOOKFILER_MODULE_UTIL_H

// C++17
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/algorithm/string.hpp>
#include <boost/system/error_code.hpp>

/*
 * bookfiler
 */
namespace bookfiler {

extern std::mutex globalMutex;

void logStatus(std::string moduleCode, std::string functionStr,
               std::string msg);
void logStatus(std::string moduleCode, std::string functionStr, std::string msg,
               boost::system::error_code ec);
void logConnectionStatus(std::string moduleCode, unsigned int,
                         std::string functionStr, std::string msg);
void logConnectionStatus(std::string moduleCode, unsigned int,
                         std::string functionStr, std::string msg,
                         boost::system::error_code ec);
std::string readFile(std::string path);

} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_UTIL_H
