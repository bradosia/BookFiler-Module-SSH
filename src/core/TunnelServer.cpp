/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.02
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "TunnelServer.hpp"

/*
 * bookfiler - ssh
 * ssh utilities. The main feature is the ssh tunnel for mysql
 */
namespace bookfiler {
namespace ssh {

TunnelServerImpl::TunnelServerImpl(
    std::map<std::string, newTunnelServerType> map_) {
  boost::system::error_code ec;
  serverState = std::make_shared<ServerState>();
  for (auto val : map_) {
    if (int *val_ = std::get_if<int>(&val.second)) {
      if (val.first == "sourcePort") {
        serverState->sourcePort = static_cast<unsigned short>(*val_);
      } else if (val.first == "destPort") {
        serverState->destPortInt = *val_;
      } else if (val.first == "remotePort") {
        serverState->remotePortInt = *val_;
      }
    } else if (double *val_ = std::get_if<double>(&val.second)) {
    } else if (std::string *val_ = std::get_if<std::string>(&val.second)) {
      if (val.first == "sourceAddress") {
        serverState->sourceAddress =
            boost::asio::ip::address_v4::from_string(*val_, ec);
        if (ec) {
          logStatus("::ForwardingServerImpl", "make_address sourceAddress", ec);
        }
      } else if (val.first == "destAddress") {
        serverState->destAddressStr = *val_;
      } else if (val.first == "remoteAddress") {
        serverState->remoteAddressStr = *val_;
      } else if (val.first == "userName") {
        serverState->userNameStr = *val_;
      } else if (val.first == "password") {
        serverState->passwordStr = *val_;
      }
    }
  }
}

TunnelServerImpl::~TunnelServerImpl() {
  for (auto &thread : threadList) {
    thread.join();
  }
}

int TunnelServerImpl::setSettingsDoc(
    std::shared_ptr<rapidjson::Value> settingsDoc_) {
  settingsDoc = settingsDoc_;
  return 0;
}

int TunnelServerImpl::run() {
  std::stringstream ss;
  auto hardwareThreadsNum = std::thread::hardware_concurrency();
#if BOOKFILER_SSH_TUNNEL_DEBUG
  ss << "threadsNum: " << serverState->threadsNum
     << "\nHardware Threads: " << hardwareThreadsNum;
  logStatus("::TunnelServerImpl::runAsync", ss.str());
#endif
  serverState->threadsNum = hardwareThreadsNum;
  // hint how much concurrency is used for the io context
  ioContext =
      std::make_shared<boost::asio::io_context>(serverState->threadsNum);
  // Create and launch a listening port
  boost::asio::spawn(
      *ioContext,
      std::bind(&ListenerImpl::run,
                std::make_shared<ListenerImpl>(
                    std::ref(*ioContext),
                    boost::asio::ip::tcp::endpoint{serverState->sourceAddress,
                                                   serverState->sourcePort},
                    boost::asio::ip::tcp::endpoint{serverState->destAddress,
                                                   serverState->destPort},
                    serverState),
                std::placeholders::_1));

  // Run the I/O service on the requested number of threads
  threadList.reserve(serverState->threadsNum);
  for (auto i = serverState->threadsNum; i > 0; --i) {
    threadList.emplace_back(&TunnelServerImpl::runIoContext, this);
  }

  return 0;
}

void TunnelServerImpl::runIoContext() { ioContext->run(); }

} // namespace ssh
} // namespace bookfiler
