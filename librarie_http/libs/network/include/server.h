#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "http_request.h"
#include "http_response.h"

namespace network {

class thread_pool;

namespace http {

class server_impl;
class request_impl;

class server {
public:
  struct request;

  server();
  server(std::shared_ptr<thread_pool> tp);
  ~server();

  bool listen(const int portno);
  bool listen_async(const int portno,
                    std::function<void(std::unique_ptr<request>)> callback);

  request accept();

  struct request {
    request(std::unique_ptr<request_impl> impl);
    request(const request &other) = delete;
    ~request();

    const http_request &get_http_request();

    bool respond(const http_response &response);
    void respond_async(http_response response,
                       std::function<void(bool)> callback = [](bool) {});

    std::unique_ptr<request_impl> get_internal();

  private:
    std::unique_ptr<request_impl> _impl;
  };

private:
  std::unique_ptr<server_impl> _impl;
};

} // namespace http
} // namespace network
