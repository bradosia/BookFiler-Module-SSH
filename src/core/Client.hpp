/*
 * @name BookFiler Module - SSH
 * @author Branden Lee
 * @version 1.00
 * @license MIT
 * @brief SSH module for BookFiler™ applications.
 */

#ifndef BOOKFILER_MODULE_SSH_CLIENT_H
#define BOOKFILER_MODULE_SSH_CLIENT_H

// config
#include "config.hpp"

// AVOID ERROR BECAUSE OF swprintf
#undef __STRICT_ANSI__
// #define FOR WIN7. CAN'T FIND sdkddkver.h ON MINGW
#define _WIN32_WINNT 0x0601

// C++
#include <utility>

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

/* boost 1.72.0
 * License: Boost Software License (similar to BSD and MIT)
 */
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

// Local Project
#include "ClientState.hpp"
#include "Util.hpp"
#include <BookFiler-Module-ssh/Interface.hpp>

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

// Please, note: The class isn't called TestSSH because I lack imagination.
// The name comes from the fact that this hasn't yet seen anything
// beyond basic testing.
/*!
 * \todo Create exceptions, instead of using logic_error
 */
class ClientImpl : public Client {
public:
  ClientImpl(std::map<std::string, newSshClientType>);
  ~ClientImpl();
  int connect();
  int command(std::string const &command);
  int send(std::string bufferStr, size_t len);
  int read(std::string &bufferStr, size_t &len);
  int commandOpenClose(std::string const &command);
  int setSettingsDoc(std::shared_ptr<rapidjson::Value> settingsDoc_);
  int tunnel();
  int forwarding();

private:
  std::shared_ptr<rapidjson::Value> settingsDoc;
  std::shared_ptr<ClientState> clientState;
  std::shared_ptr<std::thread> threadIo;
  enum state { WORK_DONE, STILL_WORKING };

  void LoopAsync(boost::function<state()> DoWork, bool isRead);
  void LoopTimer(boost::function<state()> DoWork);

  void ConnectSocket();
  void CreateSSHSession();
  state DoCreateChannel();
  state DoExecute();
  state writeCommandChannel();
  /* do commandChannelReadStr.clear() before calling
   */
  state readCommandChannel();
  /* do listenChannelReadStr.clear() before calling
   */
  state readListenChannel();
  state DoCloseChannel();
  void Cleanup();

  void runIoContext();

  std::string commandStr, commandChannelReadStr, listenChannelReadStr;
  boost::asio::io_service ioContext;
  std::shared_ptr<boost::asio::ip::tcp::socket> sockPtr;
  std::shared_ptr<boost::asio::deadline_timer> dtimerPtr;
  LIBSSH2_SESSION *session = nullptr;
  LIBSSH2_LISTENER *listener = nullptr;
  LIBSSH2_CHANNEL *commandChannel = nullptr;
  LIBSSH2_CHANNEL *listenChannel = nullptr;
  size_t writtenBytes, commandChannelReadLen, commandStrLen;
};

} // namespace ssh
} // namespace bookfiler

#endif
// end BOOKFILER_MODULE_SSH_CLIENT_H
