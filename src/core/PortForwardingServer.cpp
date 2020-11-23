/*
 * @name BookFiler Module - HTTP
 * @author Branden Lee
 * @version 1.02
 * @license MIT
 * @brief HTTP module for BookFiler™ applications.
 */

// Local Project
#include "PortForwardingServer.hpp"

/*
 * bookfiler - port
 * Port fowarding utilities. Initially this utility was made to forward mysql
 * traffic through a secure ssh connection.
 */
namespace bookfiler {
namespace port {

ForwardingServerImpl::ForwardingServerImpl(
    std::map<std::string, bookfiler::port::newForwardingServerType> map_) {
  boost::system::error_code ec;
  serverState = std::make_shared<ServerState>();
  for (auto val : map_) {
    if (int *val_ = std::get_if<int>(&val.second)) {
      if (val.first == "sourcePort") {
        serverState->sourcePort = static_cast<unsigned short>(*val_);
      } else if (val.first == "destPort") {
        serverState->destPort = static_cast<unsigned short>(*val_);
      }
    } else if (double *val_ = std::get_if<double>(&val.second)) {
    } else if (std::string *val_ = std::get_if<std::string>(&val.second)) {
      if (val.first == "sourceAddress") {
        serverState->sourceAddress = boost::asio::ip::address_v4::from_string(*val_, ec);
        if (ec) {
          logStatus("::ForwardingServerImpl", "make_address sourceAddress", ec);
        }
      } else if (val.first == "destAddress") {
        serverState->destAddress = boost::asio::ip::address_v4::from_string(*val_, ec);
        if (ec) {
          logStatus("::ForwardingServerImpl", "make_address destAddress", ec);
        }
      }
    }
  }
}

ForwardingServerImpl::~ForwardingServerImpl() {
  for (auto &thread : threadList) {
    thread.join();
  }
}

int ForwardingServerImpl::setSettingsDoc(
    std::shared_ptr<rapidjson::Value> settingsDoc_) {
  settingsDoc = settingsDoc_;
  return 0;
}

int ForwardingServerImpl::run() {
  std::stringstream ss;
  auto hardwareThreadsNum = std::thread::hardware_concurrency();
  ss << "threadsNum: " << serverState->threadsNum
     << "\nHardware Threads: " << hardwareThreadsNum;
  serverState->threadsNum = hardwareThreadsNum;
  logStatus("::ForwardingServerImpl::runAsync", ss.str());
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
    threadList.emplace_back(&ForwardingServerImpl::runIoContext, this);
  }

  return 0;
}

void ForwardingServerImpl::runIoContext() { ioContext->run(); }

} // namespace port
} // namespace bookfiler
