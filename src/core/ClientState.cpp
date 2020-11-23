/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

// Local Project
#include "ClientState.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

ClientState::ClientState() {
    remoteHostName = "127.0.0.1";
    srcHostName = "127.0.0.1";
}
ClientState::~ClientState() {}

} // namespace HTTP
} // namespace bookfiler
