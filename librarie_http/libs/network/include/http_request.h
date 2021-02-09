#pragma once

#include <cstdlib>
#include <map>
#include <sstream>
#include <string>

namespace network::http {

enum class method { GET, POST, PUT, PATCH, DELETE };

struct message_body {
  std::string content_type = "text/plain";
  std::string message;
};

struct http_request {
  method method;
  std::string uri;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  http_request() {}

  http_request(enum method met, const std::string_view u,
               const std::string_view host,
               std::unordered_map<std::string, std::string> h,
               const message_body &mb)
      : method(met), uri(u), body(mb.message) {
    headers["Host"] = host;
    headers["User-Agent"] = "Damaschin Dorin Licenta";
    headers["Content-Length"] = std::to_string(body.size());
    headers["Content-Type"] = mb.content_type;
    headers["Connection"] = "close";
    for (auto [name, value] : h) {
      headers[name] = value;
    }
  }

  http_request(const std::string &request) {
    auto met_end_pos = request.find(' ');
    if (met_end_pos == std::string::npos) {
      return;
    }
    auto met = request.substr(0, met_end_pos);
    if (met == "GET") {
      method = method::GET;
    } else if (met == "POST") {
      method = method::POST;
    } else if (met == "PUT") {
      method = method::PUT;
    } else if (met == "PATCH") {
      method = method::PATCH;
    } else if (met == "DELETE") {
      method = method::DELETE;
    } else {
      return;
    }
    auto uri_pos = request.find(' ', met_end_pos + 1);
    if (uri_pos == std::string::npos) {
      return;
    }
    uri = request.substr(met_end_pos + 1, uri_pos - met_end_pos - 1);
    auto pv_end_pos = request.find("HTTP/1.1", uri_pos);
    if (pv_end_pos == std::string::npos) {
      return;
    }
    auto body_pos = request.find("\r\n\r\n", pv_end_pos);
    if (body_pos == std::string::npos) {
      return;
    }
    auto header_start_pos = request.find("\r\n", pv_end_pos);
    if (header_start_pos == std::string::npos) {
      return;
    }
    while (header_start_pos != body_pos) {
      // TODO: mai multe verificari
      auto header_sep_pos = request.find(':', header_start_pos);
      std::string name = request.substr(header_start_pos + 2,
                                        header_sep_pos - header_start_pos - 2);
      header_start_pos = request.find("\r\n", header_sep_pos);
      std::string value = request.substr(header_sep_pos + 2,
                                         header_start_pos - header_sep_pos - 2);
      headers[name] = value;
    }
    body = request.substr(body_pos + 4);
  }

  std::string to_string() const {
    std::stringstream ss;
    switch (method) {
    case method::GET:
      ss << "GET";
      break;
    case method::POST:
      ss << "POST";
      break;
    case method::PUT:
      ss << "PUT";
      break;
    case method::PATCH:
      ss << "PATCH";
      break;
    case method::DELETE:
      ss << "DELETE";
      break;
    default:
      return "";
    };
    // prima linie HTTP
    ss << " " << uri << " "
       << "HTTP/1.1"
       << "\r\n";
    // header-e
    for (auto [name, value] : headers) {
      ss << name << ": " << value << "\r\n";
    }
    ss << "\r\n";
    // corpul mesajului
    ss << body;
    return ss.str();
  }
};

} // namespace network::http