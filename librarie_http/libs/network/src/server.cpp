#include "server.h"

#include "server_impl.h"
#include "tcp_connection.h"
#include "thread_pool.h"

namespace network::http {

server::server() : _impl(std::make_unique<server_impl>()) {}

server::server(std::shared_ptr<thread_pool> tp)
    : _impl(std::make_unique<server_impl>(tp)) {}

server::~server() = default;

// implementation

bool server::listen(const int portno) { return _impl->listen(portno); }

bool server::listen_async(
    const int portno, std::function<void(std::unique_ptr<request>)> callback) {
  return _impl->listen_async(portno, [callback = std::move(callback)](
                                         std::unique_ptr<request_impl> rq) {
    callback(std::make_unique<request>(std::move(rq)));
  });
}

server::request server::accept() { return server::request(_impl->accept()); }

server::request::request(std::unique_ptr<request_impl> impl)
    : _impl(std::move(impl)) {}

server::request::~request() = default;

const http_request &server::request::get_http_request() {
  return _impl->http_request;
}

bool server::request::respond(const http_response &response) {
  return _impl->respond(response);
}

void server::request::respond_async(http_response response,
                                    std::function<void(bool)> callback) {
  return _impl->respond_async(response, callback);
}

std::unique_ptr<request_impl> server::request::get_internal() {
  return std::move(_impl);
}

} // namespace network::http
