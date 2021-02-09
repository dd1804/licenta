#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "http_response.h"
#include "server.h"
#include "tcp_connection.h"
#include "thread_pool.h"

namespace network {
namespace http {

class request_impl;

class server_impl {
public:
  server_impl();
  server_impl(std::shared_ptr<thread_pool> tp);

  bool listen(const int portno);
  bool listen_async(const int portno, std::function<void(std::unique_ptr<request_impl>)> callback);

  std::unique_ptr<request_impl> accept();

private:
  std::shared_ptr<thread_pool> _thread_pool;
  std::shared_ptr<tcp::tcp_connection> _tcp_connection;
};

class request_impl {
public:
  http_request http_request;

  request_impl(std::shared_ptr<network::tcp::tcp_connection> conn,
               std::string data);
  request_impl(std::shared_ptr<thread_pool> tp,
               std::shared_ptr<network::tcp::tcp_connection> conn,
               std::string data);

  bool respond(const http_response &response);

  void respond_async(const http_response response,
                     std::function<void(bool)> callback);

private:
  std::shared_ptr<thread_pool> _thread_pool;
  std::shared_ptr<tcp::tcp_connection> _tcp_connection;
};

} // namespace http
} // namespace network
