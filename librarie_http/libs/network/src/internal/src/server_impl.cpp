#include "server_impl.h"

#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "tcp_connection.h"
#include "thread_pool.h"
#include "utils.h"

namespace network {
namespace http {

server_impl::server_impl()
    : _tcp_connection(std::make_shared<tcp::tcp_connection>()) {}

server_impl::server_impl(std::shared_ptr<thread_pool> tp)
    : _thread_pool(tp),
      _tcp_connection(std::make_shared<tcp::tcp_connection>()) {}

bool server_impl::listen(const int portno) {
  return _tcp_connection->listen(portno, false);
}

bool server_impl::listen_async(
    const int portno,
    std::function<void(std::unique_ptr<request_impl>)> callback) {

  auto tp = _thread_pool;
  return _tcp_connection->listen_async(
      portno, tp,
      [tp, callback = std::move(callback)](
          std::variant<std::shared_ptr<tcp::tcp_connection>,
                       tcp::tcp_connection::error>
              conn) {
        if (conn.index() == tcp::tcp_connection::SUCCESS_IND) {
          auto tcp_conn = std::get<std::shared_ptr<tcp::tcp_connection>>(conn);
          tcp_conn->set_receive_callback(
              tp, false, utils::http::IsHttpMessageComplete,
              [tp, tcp_conn, callback = std::move(callback)](
                  std::variant<std::string, tcp::tcp_connection::error>
                      message) {
                if (message.index() == tcp::tcp_connection::SUCCESS_IND) {
                  callback(std::make_unique<request_impl>(
                      tp, tcp_conn, std::move(std::get<std::string>(message))));
                }
              });
        }
      });
}

std::unique_ptr<request_impl> server_impl::accept() {
  std::shared_ptr<tcp::tcp_connection> tcp_conn;
  for (;;) {
    auto response = _tcp_connection->accept();
    if (response.index() == tcp::tcp_connection::ERR_IND) {
      _tcp_connection->close();
      utils::err::log("accept: eroare stabilire conexiune");
      continue;
    }

    tcp_conn = std::make_shared<tcp::tcp_connection>(
        std::get<tcp::tcp_connection>(response));

    auto data = tcp_conn->receive();
    if (data.index() == tcp::tcp_connection::ERR_IND) {
      _tcp_connection->close();
      utils::err::log("accept: eroare citire request");
      continue;
    }

    return std::make_unique<request_impl>(tcp_conn,
                                          std::get<std::string>(data));
  }
}

request_impl::request_impl(std::shared_ptr<network::tcp::tcp_connection> conn,
                           std::string data)
    : _tcp_connection(conn), http_request(data) {}

request_impl::request_impl(std::shared_ptr<thread_pool> tp,
                           std::shared_ptr<network::tcp::tcp_connection> conn,
                           std::string data)
    : _thread_pool(tp), _tcp_connection(conn), http_request(data) {}

bool request_impl::respond(const http_response &response) {
  // utils::debug::log("trimite raspuns:");
  // utils::debug::log(response.to_string());
  bool success = true;
  if (_tcp_connection->send(response.to_string()) == false) {
    success = false;
  }
  if (_tcp_connection->close() == false) {
    success = false;
  }
  return success;
}

void request_impl::respond_async(const http_response response,
                                 std::function<void(bool)> callback) {

  // utils::debug::log("trimite raspuns:");
  // utils::debug::log(response.to_string());
  auto tp = _thread_pool;
  auto tcp_conn = _tcp_connection;

  tcp_conn->set_send_callback(response.to_string(), tp, true,
                              [tp, tcp_conn, callback](bool success) {
                                if (success) {
                                  callback(true);
                                } else {
                                  callback(false);
                                }
                              });
}

} // namespace http
} // namespace network
