#include "client.h"
#include "client_impl.h"
#include "thread_pool.h"

namespace network::http {

// constructors

client::client() : _impl(std::make_unique<client_impl>()) {}

client::client(std::shared_ptr<thread_pool> tp)
    : _impl(std::make_unique<client_impl>(tp)) {}

client::~client() = default;

// implementation

http_response client::get(const std::string_view url) {
  return _impl->send_request(method::GET, url,
                             std::unordered_map<std::string, std::string>(),
                             message_body());
}

http_response
client::get(const std::string_view url,
            const std::unordered_map<std::string, std::string> &headers) {
  return _impl->send_request(method::GET, url, headers, message_body());
}

void client::get_async(const std::string_view url,
                       std::function<void(http_response)> callback) {
  _impl->send_request_async(method::GET, url,
                            std::unordered_map<std::string, std::string>(),
                            message_body(), callback);
}

void client::get_async(
    const std::string_view url,
    const std::unordered_map<std::string, std::string> &headers,
    std::function<void(http_response)> callback) {
  _impl->send_request_async(method::GET, url, headers, message_body(),
                            callback);
}

http_response client::post(const std::string_view url,
                           const message_body &message_body) {
  return _impl->send_request(method::POST, url,
                             std::unordered_map<std::string, std::string>(),
                             message_body);
}

http_response
client::post(const std::string_view url,
             const std::unordered_map<std::string, std::string> &headers,
             const message_body &message_body) {
  return _impl->send_request(method::POST, url, headers, message_body);
}

void client::post_async(const std::string_view url,
                        const message_body &&message_body,
                        std::function<void(http_response)> callback) {
  _impl->send_request_async(method::POST, url,
                            std::unordered_map<std::string, std::string>(),
                            std::move(message_body), callback);
}

void client::post_async(
    const std::string_view url,
    const std::unordered_map<std::string, std::string> &headers,
    const message_body &&message_body,
    std::function<void(http_response)> callback) {
  _impl->send_request_async(method::POST, url, headers, std::move(message_body),
                            callback);
}

http_response client::put(const std::string_view url,
                          const message_body &message_body) {
  return _impl->send_request(method::PUT, url,
                             std::unordered_map<std::string, std::string>(),
                             message_body);
}

http_response
client::put(const std::string_view url,
            const std::unordered_map<std::string, std::string> &headers,
            const message_body &message_body) {
  return _impl->send_request(method::PUT, url, headers, message_body);
}

void client::put_async(const std::string_view url,
                       const message_body &&message_body,
                       std::function<void(http_response)> callback) {
  _impl->send_request_async(method::PUT, url,
                            std::unordered_map<std::string, std::string>(),
                            std::move(message_body), callback);
}

void client::put_async(
    const std::string_view url,
    const std::unordered_map<std::string, std::string> &headers,
    const message_body &&message_body,
    std::function<void(http_response)> callback) {
  _impl->send_request_async(method::PUT, url, headers, std::move(message_body),
                            callback);
}

http_response client::patch(const std::string_view url,
                            const message_body &message_body) {
  return _impl->send_request(method::PATCH, url,
                             std::unordered_map<std::string, std::string>(),
                             message_body);
}

http_response
client::patch(const std::string_view url,
              const std::unordered_map<std::string, std::string> &headers,
              const message_body &message_body) {
  return _impl->send_request(method::PATCH, url, headers, message_body);
}

void client::patch_async(const std::string_view url,
                         const message_body &&message_body,
                         std::function<void(http_response)> callback) {
  _impl->send_request_async(method::PATCH, url,
                            std::unordered_map<std::string, std::string>(),
                            std::move(message_body), callback);
}

void client::patch_async(
    const std::string_view url,
    const std::unordered_map<std::string, std::string> &headers,
    const message_body &&message_body,
    std::function<void(http_response)> callback) {
  _impl->send_request_async(method::PATCH, url, headers,
                            std::move(message_body), callback);
}

http_response client::del(const std::string_view url) {
  return _impl->send_request(method::DELETE, url,
                             std::unordered_map<std::string, std::string>(),
                             message_body());
}

http_response
client::del(const std::string_view url,
            const std::unordered_map<std::string, std::string> &headers) {
  return _impl->send_request(method::DELETE, url, headers, message_body());
}

void client::del_async(const std::string_view url,
                       std::function<void(http_response)> callback) {
  _impl->send_request_async(method::DELETE, url,
                            std::unordered_map<std::string, std::string>(),
                            message_body(), callback);
}

void client::del_async(
    const std::string_view url,
    const std::unordered_map<std::string, std::string> &headers,
    std::function<void(http_response)> callback) {
  _impl->send_request_async(method::DELETE, url, headers, message_body(),
                            callback);
}

} // namespace network::http
