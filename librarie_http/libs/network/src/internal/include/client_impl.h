#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "http_request.h"
#include "http_response.h"
#include "thread_pool.h"
#include "url.h"

namespace network {
namespace http {

class client_impl {
public:
  client_impl();
  client_impl(std::shared_ptr<thread_pool>);

  http_response
  send_request(method met, const url &u,
               const std::unordered_map<std::string, std::string> &headers,
               const message_body &message_body);

  void send_request_async(
      method met, const url &u,
      const std::unordered_map<std::string, std::string> &headers,
      const message_body &&message_body,
      std::function<void(http_response)> callback);

private:
  std::shared_ptr<thread_pool> _thread_pool;
};

} // namespace http
} // namespace network
