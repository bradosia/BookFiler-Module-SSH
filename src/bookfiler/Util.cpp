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
 * bookfiler - HTTP
 */
namespace bookfiler {

std::mutex globalMutex;

void logStatus(std::string moduleCode, std::string functionStr, std::string msg) {
  const std::lock_guard<std::mutex> lock(globalMutex);
  std::cout << "\n=== THREAD " << std::this_thread::get_id() << " ===\n"
            << moduleCode << functionStr << " " << msg << std::endl;
}

void logStatus(std::string moduleCode,std::string functionStr, std::string msg,
               boost::system::error_code ec) {
  const std::lock_guard<std::mutex> lock(globalMutex);
  std::cout << "\n=== THREAD " << std::this_thread::get_id() << " ===\n"
            << moduleCode << functionStr << " " << msg << " ERROR:\n"
            << ec.message() << std::endl;
}

void logConnectionStatus(std::string moduleCode,unsigned int socketId, std::string functionStr,
                         std::string msg) {
  const std::lock_guard<std::mutex> lock(globalMutex);
  std::cout << "\n=== THREAD " << std::this_thread::get_id() << ", conn #"
            << socketId << " ===\n"
            << moduleCode << functionStr << " " << msg << std::endl;
}

void logConnectionStatus(std::string moduleCode,unsigned int socketId, std::string functionStr,
                         std::string msg, boost::system::error_code ec) {
  const std::lock_guard<std::mutex> lock(globalMutex);
  std::cout << "\n=== THREAD " << std::this_thread::get_id() << ", conn #"
            << socketId << " ===\n"
            << moduleCode << functionStr << " " << msg << " ERROR:\n"
            << ec.message() << std::endl;
}

std::string readFile(std::string path) {
  std::string ret;
  if (auto const fd = std::fopen(path.c_str(), "rb")) {
    auto const bytes = std::filesystem::file_size(path);
    ret.resize(bytes);
    std::fread(ret.data(), 1, bytes, fd);
    std::fclose(fd);
  }
  return ret;
}

} // namespace bookfiler
