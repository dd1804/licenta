#pragma once

#include <functional>
#include <map>
#include <memory>

#include "http_request.h"
#include "http_response.h"

namespace network {

class thread_pool;

namespace http {

class client_impl;

class client {
public:
  client();
  client(std::shared_ptr<thread_pool>);
  ~client();

  http_response get(const std::string_view url);
  http_response
  get(const std::string_view url,
      const std::unordered_map<std::string, std::string> &headers);
  void get_async(const std::string_view url,
                 std::function<void(http_response)> callback);
  void get_async(const std::string_view url,
                 const std::unordered_map<std::string, std::string> &headers,
                 std::function<void(http_response)> callback);

  http_response post(const std::string_view url, const message_body &body);
  http_response
  post(const std::string_view url,
       const std::unordered_map<std::string, std::string> &headers,
       const message_body &body);
  void post_async(const std::string_view url, const message_body &&message_body,
                  std::function<void(http_response)> callback);
  void post_async(const std::string_view url,
                  const std::unordered_map<std::string, std::string> &headers,
                  const message_body &&message_body,
                  std::function<void(http_response)> callback);

  http_response put(const std::string_view url,
                    const message_body &message_body);
  http_response put(const std::string_view url,
                    const std::unordered_map<std::string, std::string> &headers,
                    const message_body &message_body);
  void put_async(const std::string_view url, const message_body &&message_body,
                 std::function<void(http_response)> callback);
  void put_async(const std::string_view url,
                 const std::unordered_map<std::string, std::string> &headers,
                 const message_body &&message_body,
                 std::function<void(http_response)> callback);

  http_response patch(const std::string_view url,
                      const message_body &message_body);
  http_response
  patch(const std::string_view url,
        const std::unordered_map<std::string, std::string> &headers,
        const message_body &message_body);
  void patch_async(const std::string_view url,
                   const message_body &&message_body,
                   std::function<void(http_response)> callback);
  void patch_async(const std::string_view url,
                   const std::unordered_map<std::string, std::string> &headers,
                   const message_body &&message_body,
                   std::function<void(http_response)> callback);

  http_response del(const std::string_view url);
  http_response
  del(const std::string_view url,
      const std::unordered_map<std::string, std::string> &headers);
  void del_async(const std::string_view url,
                 std::function<void(http_response)> callback);
  void del_async(const std::string_view url,
                 const std::unordered_map<std::string, std::string> &headers,
                 std::function<void(http_response)> callback);

  // http_response post(const std::string &, const std::string &,
  //                    const std::string &);
  // http_response post(const std::string &,
  //                    const std::map<std::string, std::string> &,
  //                    const std::string &, const std::string &);

  // http_response put(const std::string &, const std::string &,
  //                   const std::string &);

  // http_response put(const std::string &,
  //                   const std::map<std::string, std::string> &,
  //                   const std::string &, const std::string &);

  // http_response patch(const std::string &, const std::string &,
  //                     const std::string &);
  // http_response patch(const std::string &,
  //                     const std::map<std::string, std::string> &,
  //                     const std::string &, const std::string &);

  // http_response del(const std::string &);

private:
  std::unique_ptr<client_impl> _impl;
};

} // namespace http
} // namespace network
