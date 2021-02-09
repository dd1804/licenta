#pragma once

#include <iostream>
#include <string>

namespace utils {

class err {
public:
  static void log(const std::string_view message) {
    std::cout << "ERROR: " << message << std::endl;
  }
};

class debug {
public:
  static void log(const std::string_view message) {
#ifdef DEBUG
    std::cout << "DEBUG: " << message << std::endl;
#endif
  }
};

class http {
public:
  static bool IsHttpMessageComplete(std::string_view message) {
    auto end_of_header_pos = message.find("\r\n\r\n");
    if (end_of_header_pos != std::string::npos) {
      // std::cout << "!!! end_of_header_pos = " << end_of_header_pos <<
      // std::endl;
      auto content_length_pos = message.find("Content-Length: ");
      if (content_length_pos != std::string::npos) {
        auto content_length_start = content_length_pos + 16;
        // //std::cout << "!!! content_length_start = " << content_length_start
        //           << std::endl;
        auto content_length_end = message.find("\r\n", content_length_pos);
        // std::cout << "!!! content_length_end = " << content_length_end
        //           << std::endl;
        if (content_length_end != std::string::npos) {
          auto declared_content_length =
              std::atoi(message
                            .substr(content_length_start,
                                    content_length_end - content_length_start)
                            .data());
          // std::cout << "!!! declared_content_length = "
          //           << declared_content_length << std::endl;
          auto message_body_start = end_of_header_pos + 4;
          // std::cout << "!!! message_body_start = " << message_body_start
          //           << std::endl;
          // std::cout << "!!! message.size() = " << message.size() <<
          // std::endl;
          if (message.size() - message_body_start >= declared_content_length) {
            return true;
          }
        }
      }
    }
    return false;
  }
};

} // namespace utils
