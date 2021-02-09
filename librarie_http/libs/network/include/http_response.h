#pragma once

#include <cstdlib>
#include <map>
#include <sstream>
#include <string>

#include "http_request.h"
#include "time.h"

namespace network::http {

enum class status {
  OK /* 200 */,
  NOT_FOUND /* 404 */,
  NOT_IMPLEMENTED /* 501 */
};

struct http_response {
  int status_code;
  std::map<std::string, std::string> headers;
  std::string body;

  http_response() : status_code(-1) {}

  http_response(const std::string &response) {
    auto start_pos = response.find("HTTP/1.1");
    if (start_pos == std::string::npos) {
      status_code = -1;
      return;
    }
    auto sc_end_pos = response.find(' ', start_pos + 9);
    if (start_pos == std::string::npos) {
      status_code = -1;
      return;
    }
    status_code = atoi(
        response.substr(start_pos + 9, sc_end_pos - start_pos - 9).c_str());
    auto body_pos = response.find("\r\n\r\n", sc_end_pos);
    if (body_pos == std::string::npos) {
      status_code = -1;
      return;
    }
    auto header_start_pos = response.find("\r\n", sc_end_pos);
    if (header_start_pos == std::string::npos) {
      status_code = -1;
      return;
    }
    while (header_start_pos != body_pos) {
      // TODO: mai multe verificari
      auto header_sep_pos = response.find(':', header_start_pos);
      std::string name = response.substr(header_start_pos + 2,
                                         header_sep_pos - header_start_pos - 2);
      header_start_pos = response.find("\r\n", header_sep_pos);
      std::string value = response.substr(
          header_sep_pos + 2, header_start_pos - header_sep_pos - 2);
      headers[name] = value;
    }
    body = response.substr(body_pos + 4);
  }

  http_response(status st, message_body message_body = {}) {
    switch (st) {
    case status::OK:
      status_code = 200;
      break;
    case status::NOT_FOUND:
      status_code = 404;
      break;
    case status::NOT_IMPLEMENTED:
      status_code = 501;
      break;
    default:
      break;
    }

    headers["Connection"] = "close";
    headers["Server"] = "Dorin Damaschin Licenta";
    if (!message_body.message.empty()) {
      body = message_body.message;
      headers["Content-Length"] = std::to_string(body.size());
      headers["Content-Type"] = message_body.content_type;
    } else {
      headers["Content-Length"] = std::to_string(0);
    }
    char buf[100];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    headers["Date"] = buf;
  }

  std::string to_string() const {
    std::string description;
    switch (status_code) {
    case 200:
      description = "OK";
      break;
    case 404:
      description = "Not Found";
      break;
    case 501:
      description = "Not Implemented";
      break;

    default:
      break;
    };
    std::stringstream ss;
    ss << "HTTP/1.1" << ' ' << status_code << ' ' << description << "\r\n";
    for (auto [name, value] : headers) {
      ss << name << ": " << value << "\r\n";
    }
    ss << "\r\n";
    ss << body;
    return ss.str();
  }
};

} // namespace network::http