/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_CONFIG_H
#define BOOKFILER_MODULE_SSH_CONFIG_H

#define BOOKFILER_SSH_TUNNEL_DEBUG 0

// C++
#include <string>

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#define BOOST_ALLOW_DEPRECATED_HEADERS

namespace bookfiler {
namespace port {

static std::string moduleName = "BookFiler Module Port";
static std::string moduleCode = "bookfiler::port";

} // namespace port

namespace ssh {

static std::string moduleName = "BookFiler Module SSH";
static std::string moduleCode = "bookfiler::ssh";

} // namespace port
} // namespace bookfiler

#endif // BOOKFILER_MODULE_SSH_CONFIG_H
