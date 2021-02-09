#include "client_impl.h"

#include "http_request.h"
#include "tcp_connection.h"
#include "utils.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <strings.h>

namespace network::http {

// contructors

client_impl::client_impl() {}

client_impl::client_impl(std::shared_ptr<thread_pool> tp) : _thread_pool(tp) {}

// implementation

http_response client_impl::send_request(
    method met, const url &u,
    const std::unordered_map<std::string, std::string> &headers,
    const message_body &message_body) {

  http_request request(met, u.path, u.domain, headers, message_body);
  utils::debug::log("am creat request:\n" + request.to_string() + "\n");
  tcp::tcp_connection connection;
  if (connection.connect(u) == false) {
    return http_response();
  }
  if (connection.send(request.to_string()) == false) {
    return http_response();
  }
  auto response = connection.receive();
  if (response.index() == tcp::tcp_connection::ERR_IND) {
    return http_response();
  }

  return http_response(std::get<std::string>(response));
}

void client_impl::send_request_async(
    method met, const url &u,
    const std::unordered_map<std::string, std::string> &headers,
    const message_body &&message_body,
    std::function<void(http_response)> callback) {

  auto thread_pool = _thread_pool;
  http_request request(met, u.path, u.domain, headers, message_body);
  // utils::debug::log("am creat request-ul:\n" + request.to_string() + "\n");
  auto connection = std::make_shared<tcp::tcp_connection>();
  if (connection->get_socket(true, thread_pool)) {
    connection->set_send_callback(
        request.to_string(), thread_pool, false, [callback](bool success) {
          if (!success) {
            utils::err::log("send_request_async: eroare scriere");
            callback(http_response());
          }
        });
    connection->set_receive_callback(
        thread_pool, true, utils::http::IsHttpMessageComplete,
        [callback](
            std::variant<std::string, tcp::tcp_connection::error> response) {
          if (response.index() == tcp::tcp_connection::SUCCESS_IND) {
            callback(http_response(std::get<std::string>(response)));
          } else {
            utils::err::log("receive_request_async: eroare citire");
            callback(http_response());
          }
        });

    connection->connect_async(u, thread_pool);
  }
}

} // namespace network::http
