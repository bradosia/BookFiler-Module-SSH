/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.01
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "Client.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

ClientImpl::ClientImpl(
    std::map<std::string, bookfiler::ssh::newSshClientType> map_) {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::ClientImpl", "START");
#endif
  clientState = std::make_shared<ClientState>();
  dtimerPtr = std::make_shared<boost::asio::deadline_timer>(ioContext);
  sockPtr = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
  for (auto val : map_) {
    if (int *val_ = std::get_if<int>(&val.second)) {
      if (val.first == "port") {
        clientState->port = static_cast<unsigned short>(*val_);
      } else if (val.first == "remotePort") {
        clientState->remotePort = *val_;
      } else if (val.first == "srcPort") {
        clientState->srcPort = *val_;
      }
    } else if (double *val_ = std::get_if<double>(&val.second)) {
    } else if (std::string *val_ = std::get_if<std::string>(&val.second)) {
      if (val.first == "userName") {
        clientState->userName = *val_;
      } else if (val.first == "password") {
        clientState->password = *val_;
      } else if (val.first == "hostName") {
        clientState->hostName = *val_;
      } else if (val.first == "remoteHostName") {
        clientState->remoteHostName = *val_;
      } else if (val.first == "srcHostName") {
        clientState->srcHostName = *val_;
      }
    }
  }
}

ClientImpl::~ClientImpl() {
  if (commandChannel != nullptr) {
    libssh2_channel_free(commandChannel);
    commandChannel = nullptr;
  }

  if (listenChannel != nullptr) {
    libssh2_channel_free(listenChannel);
    listenChannel = nullptr;
  }

  if (listener != nullptr) {
    libssh2_channel_forward_cancel(listener);
    listener = nullptr;
  }

  if (session != nullptr) {
    libssh2_session_set_blocking(session, 1);
    libssh2_session_disconnect(session, "Disconnecting");
    libssh2_session_free(session);
    session = nullptr;
  }

  if (sockPtr->is_open()) {
    try {
      boost::system::error_code ec;
      sockPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      sockPtr->close(ec);
    } catch (std::exception &ex) {
      // NO EXCEPTIONS IN DTOR, LOG IT SOMEWHERE
    }
  }
}

int ClientImpl::setSettingsDoc(std::shared_ptr<rapidjson::Value> settingsDoc_) {
  settingsDoc = settingsDoc_;
  return 0;
}

int ClientImpl::connect() {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::connect", "START");
#endif
  // THESE ARE BLOCKING
  ConnectSocket();
  CreateSSHSession();
  int rc = 0;

#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::DoHandshake", "START");
#endif
  rc = libssh2_session_handshake(session, sockPtr->native_handle());
  if (rc) {
    logStatus("::ClientImpl::connect", "libssh2_session_handshake ERROR");
    return -1;
  }

  threadIo = std::make_shared<std::thread>(&ClientImpl::runIoContext, this);

#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::DoAuthenticate", "START");
#endif
  rc = libssh2_userauth_password(session, clientState->userName.c_str(),
                                 clientState->password.c_str());
  if (rc) {
    logStatus("::ClientImpl::connect", "libssh2_userauth_password ERROR");
    return -1;
  }

  return 0;
}

void ClientImpl::runIoContext() { ioContext.run(); }

/*!
 * \todo Turn this into a loop
 */
int ClientImpl::command(std::string const &commandStr_) {
  commandStr = commandStr_;

  ioContext.reset();
  sockPtr->async_read_some(boost::asio::null_buffers(),
                           boost::bind(&ClientImpl::LoopAsync, this,
                                       boost::protect(boost::bind(
                                           &ClientImpl::DoCreateChannel, this)),
                                       true));
  sockPtr->async_write_some(
      boost::asio::null_buffers(),
      boost::bind(
          &ClientImpl::LoopAsync, this,
          boost::protect(boost::bind(&ClientImpl::DoCreateChannel, this)),
          false));
  ioContext.run();

  ioContext.reset();
  sockPtr->async_read_some(
      boost::asio::null_buffers(),
      boost::bind(&ClientImpl::LoopAsync, this,
                  boost::protect(boost::bind(&ClientImpl::DoExecute, this)),
                  true));
  sockPtr->async_write_some(
      boost::asio::null_buffers(),
      boost::bind(&ClientImpl::LoopAsync, this,
                  boost::protect(boost::bind(&ClientImpl::DoExecute, this)),
                  false));
  ioContext.run();

  commandChannelReadStr.clear();
  ioContext.reset();
  sockPtr->async_read_some(
      boost::asio::null_buffers(),
      boost::bind(
          &ClientImpl::LoopAsync, this,
          boost::protect(boost::bind(&ClientImpl::readCommandChannel, this)),
          true));
  sockPtr->async_write_some(
      boost::asio::null_buffers(),
      boost::bind(
          &ClientImpl::LoopAsync, this,
          boost::protect(boost::bind(&ClientImpl::readCommandChannel, this)),
          false));
  ioContext.run();
  std::stringstream ss;
  ss << "channelReadStr:\n" << commandChannelReadStr << std::endl;
  logStatus("::ClientImpl::command", ss.str());

  ioContext.reset();
  sockPtr->async_read_some(boost::asio::null_buffers(),
                           boost::bind(&ClientImpl::LoopAsync, this,
                                       boost::protect(boost::bind(
                                           &ClientImpl::DoCloseChannel, this)),
                                       true));
  sockPtr->async_write_some(boost::asio::null_buffers(),
                            boost::bind(&ClientImpl::LoopAsync, this,
                                        boost::protect(boost::bind(
                                            &ClientImpl::DoCloseChannel, this)),
                                        false));
  ioContext.run();

  // THIS IS BLOCKING
  Cleanup();
  return 0;
}

int ClientImpl::send(std::string bufferStr, size_t len) {
  commandStr = bufferStr;
  commandStrLen = len;
  writtenBytes = 0;

  writeCommandChannel();

  return 0;
}

int ClientImpl::read(std::string &bufferStr, size_t &len) {
  commandChannelReadStr.clear();
  commandChannelReadLen = 0;

  readCommandChannel();
#if BOOKFILER_SSH_TUNNEL_DEBUG
  std::stringstream ss;
  ss << "channelReadStr:\n" << commandChannelReadStr << std::endl;
  // not realiable because of \0
  logStatus("::ClientImpl::read", ss.str());
#endif
  bufferStr = commandChannelReadStr;
  len = commandChannelReadLen;

  return 0;
}

void ClientImpl::LoopAsync(boost::function<state()> DoWork, bool isRead) {
  if (ioContext.stopped()) {
    return;
  }

  state st = DoWork();

  if (st == STILL_WORKING) {
    if (isRead) {
      sockPtr->async_read_some(
          boost::asio::null_buffers(),
          boost::bind(&ClientImpl::LoopAsync, this, DoWork, isRead));
    } else {
      sockPtr->async_write_some(
          boost::asio::null_buffers(),
          boost::bind(&ClientImpl::LoopAsync, this, DoWork, isRead));
    }
    return;
  }

  // OTHERWISE, WORK IS OVER
  ioContext.stop();
}

void ClientImpl::LoopTimer(boost::function<state()> DoWork) {
  state st = DoWork();

  if (st == STILL_WORKING) {
    dtimerPtr->expires_from_now(boost::posix_time::milliseconds(500));
    dtimerPtr->async_wait(boost::bind(&ClientImpl::LoopTimer, this, DoWork));
  }
  // OTHERWISE, WORK IS OVER; NOTHING TO DO, AS THE TIMER IS NOT REARMED
}

void ClientImpl::ConnectSocket() {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::ConnectSocket", "START");
#endif
  boost::asio::ip::tcp::resolver rsv(ioContext);
#if BOOKFILER_SSH_TUNNEL_DEBUG
  std::stringstream ss;
  ss << "query SETTINGS:"
     << "\nhostName: " << clientState->hostName
     << "\nport: " << clientState->port << std::endl;
  logStatus("::ClientImpl::ConnectSocket", ss.str());
#endif
  boost::asio::ip::tcp::resolver::query query(
      clientState->hostName,
      boost::lexical_cast<std::string>(clientState->port));
  boost::asio::ip::tcp::resolver::iterator iter = rsv.resolve(query);
  boost::asio::connect(*sockPtr, iter);
}

void ClientImpl::CreateSSHSession() {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::CreateSSHSession", "START");
#endif
  session = libssh2_session_init();
  if (!session) {
    throw std::logic_error("Error creating session");
  }

  libssh2_session_set_blocking(session, 1);
}

ClientImpl::state ClientImpl::DoCreateChannel() {
  commandChannel = libssh2_channel_open_session(session);

  if (commandChannel == nullptr) {
    int rc = libssh2_session_last_error(session, nullptr, nullptr, 0);

    if (rc == LIBSSH2_ERROR_EAGAIN) {
      return STILL_WORKING;
    } else {
      throw std::logic_error("Error opening channel");
    }
  }

  return WORK_DONE;
}

ClientImpl::state ClientImpl::DoExecute() {
  int rc = libssh2_channel_exec(commandChannel, commandStr.c_str());

  if (rc == LIBSSH2_ERROR_EAGAIN) {
    return STILL_WORKING;
  }

  if (rc) {
    throw std::logic_error("Error executing command");
  }

  return WORK_DONE;
}

ClientImpl::state ClientImpl::writeCommandChannel() {
  int rc = 0;
  writtenBytes = 0;
  do {
    rc =
        libssh2_channel_write(commandChannel, commandStr.data(), commandStrLen);

    if (rc < 0) {
      logStatus("::ClientImpl::writeCommandChannel",
                "libssh2_channel_write ERROR");
      return STILL_WORKING;
    }

    size_t writtenBytesLast = writtenBytes;
    writtenBytes += rc;
#if BOOKFILER_SSH_TUNNEL_DEBUG
    std::stringstream ss;
    ss << "written [" << writtenBytes << " of " << commandStr.size()
       << " byte]:\n"
       << commandStr.substr(writtenBytesLast, writtenBytes) << std::endl;
    logStatus("::ClientImpl::writeCommandChannel", ss.str());
#endif
  } while (rc > 0 && writtenBytes < commandStrLen);

  return WORK_DONE;
}

ClientImpl::state ClientImpl::readCommandChannel() {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::readCommandChannel", "START");
#endif
  char buffer[0x4001];
  int rc = 0;
  commandChannelReadStr.clear();
  commandChannelReadLen = 0;

  do {
    int rc = libssh2_channel_read(commandChannel, buffer, sizeof(buffer));

    if (rc < 0) {
      logStatus("::ClientImpl::readCommandChannel",
                "libssh2_channel_read ERROR");
      return STILL_WORKING;
    }

    commandChannelReadStr.append(buffer, rc);
    commandChannelReadLen += rc;
#if BOOKFILER_SSH_TUNNEL_DEBUG
    std::stringstream ss;
    ss << "read [" << rc << " bytes]:"
       << "\nBytes: ";
    unsigned int limit = std::min(1000, rc);
    for (unsigned int i = 0; i < limit; i++) {
      ss << buffer[i] << " ";
      // ss << std::setw(2) << std::setfill('0') << std::hex << buffer[i] << "
      // ";
    }
    ss << std::endl;
    logStatus("::ClientImpl::readCommandChannel", ss.str());
#endif
  } while (rc > 0);

  return WORK_DONE;
}

ClientImpl::state ClientImpl::readListenChannel() {
  char buffer[0x4001];
  int rc = libssh2_channel_read(listenChannel, buffer, sizeof(buffer) - 1);

  if (rc > 0) {
    // BUFFER IS NOT AN SZ-std::string, SO WE TURN IT INTO ONE
    buffer[rc] = '\0';
    listenChannelReadStr.append(buffer);

    std::stringstream ss;
    ss << "buffer:\n" << buffer << std::endl;
    logStatus("::ClientImpl::readListenChannel", ss.str());

    // WE'RE NOT FINISHED UNTIL WE READ 0
    return STILL_WORKING;
  }

  if (rc == LIBSSH2_ERROR_EAGAIN) {
    return STILL_WORKING;
  }

  if (rc) {
    throw std::logic_error("Error getting execution result");
  }

  return WORK_DONE;
}

ClientImpl::state ClientImpl::DoCloseChannel() {
  int rc = libssh2_channel_close(commandChannel);

  if (rc == LIBSSH2_ERROR_EAGAIN) {
    return STILL_WORKING;
  }

  if (rc) {
    throw std::logic_error("Error closing channel");
  }

  return WORK_DONE;
}

void ClientImpl::Cleanup() {
  int exitcode = libssh2_channel_get_exit_status(commandChannel);
  char *buf =
      const_cast<char *>("A buffer for getting the exit signal, if any");
  libssh2_channel_get_exit_signal(commandChannel, &buf, nullptr, nullptr,
                                  nullptr, nullptr, nullptr);

  if (buf == nullptr) {
    std::cout << "No signal" << std::endl;
  } else {
    std::string str = buf;
    std::cout << (str.length() == 0 ? "No signal description" : str)
              << std::endl;
  }

  libssh2_channel_free(commandChannel);
  commandChannel = nullptr;
}

int ClientImpl::forwarding() {
  logStatus("::ClientImpl::forwarding", "START");

  // Start blocking API
  libssh2_session_set_blocking(session, 1);

  listener = libssh2_channel_forward_listen_ex(
      session, clientState->remoteHostName.c_str(), clientState->remotePort,
      &clientState->remoteBoundPort, 1);
  if (!listener) {
    logStatus("::ClientImpl::tunnel",
              "libssh2_channel_forward_listen_ex ERROR:\nCould not start the "
              "tcpip-forward listener!\n"
              "(Note that this can be a problem at the server!"
              " Please review the server logs.)\n");
    return -1;
  }

  logStatus("::ClientImpl::tunnel",
            "libssh2_channel_forward_accept WAITING...");
  listenChannel = libssh2_channel_forward_accept(listener);
  if (!listenChannel) {
    logStatus(
        "::ClientImpl::tunnel",
        "libssh2_channel_forward_accept ERROR:\nCould not accept connection!\n"
        "(Note that this can be a problem at the server!"
        " Please review the server logs.)\n");
    return -1;
  }

  // Stop blocking API
  libssh2_session_set_blocking(session, 0);

  listenChannelReadStr.clear();
  ioContext.reset();
  sockPtr->async_read_some(
      boost::asio::null_buffers(),
      boost::bind(
          &ClientImpl::LoopAsync, this,
          boost::protect(boost::bind(&ClientImpl::readListenChannel, this)),
          true));
  sockPtr->async_write_some(
      boost::asio::null_buffers(),
      boost::bind(
          &ClientImpl::LoopAsync, this,
          boost::protect(boost::bind(&ClientImpl::readListenChannel, this)),
          false));
  ioContext.run();
  std::stringstream ss;
  ss << "channelReadStr:\n" << listenChannelReadStr << std::endl;
  logStatus("::ClientImpl::command", ss.str());

  return 0;
}

int ClientImpl::tunnel() {
#if BOOKFILER_SSH_TUNNEL_DEBUG
  logStatus("::ClientImpl::tunnel", "START");
#endif

  std::stringstream ss;
  ss << "libssh2_channel_direct_tcpip_ex SETTINGS:"
     << "\nThird party host: " << clientState->remoteHostName
     << "\nPort on third party: " << clientState->remotePort
     << "\nHost to tell the SSH server: " << clientState->srcHostName
     << "\nPort to tell the SSH server: " << clientState->srcPort << std::endl;
  logStatus("::ClientImpl::tunnel", ss.str());
  commandChannel = libssh2_channel_direct_tcpip_ex(
      session, clientState->remoteHostName.c_str(), clientState->remotePort,
      clientState->srcHostName.c_str(), clientState->srcPort);
  if (!commandChannel) {
    std::stringstream ss;
    logStatus("::ClientImpl::tunnel", "libssh2_channel_direct_tcpip_ex ERROR");
    return -1;
  }

  return 0;
}

} // namespace ssh
} // namespace bookfiler
