#pragma once

#include "thread_pool.h"
#include "url.h"

#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace network::tcp {

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
  tcp_connection();
  tcp_connection(int sockfd, uint32_t unique_id);
  tcp_connection(const tcp_connection &other);

  struct error {};
  constexpr static int SUCCESS_IND = 0;
  constexpr static int ERR_IND = 1;

  bool get_socket(bool non_blocking, std::shared_ptr<thread_pool> tp);

  bool connect(const url &serv_url);
  void connect_async(const url &serv_url, std::shared_ptr<thread_pool> tp);

  bool listen(const int portno, bool non_blocking);
  bool listen_async(
      const int portno, std::shared_ptr<thread_pool> tp,
      std::function<void(std::variant<std::shared_ptr<tcp_connection>, error>)>
          callback);

  std::variant<tcp_connection, error> accept();

  bool send(const std::string_view message);
  void set_send_callback(std::string message, std::shared_ptr<thread_pool> tp,
                         bool cls, std::function<void(bool)> callback);

  std::variant<std::string, error> receive();
  void set_receive_callback(
      std::shared_ptr<thread_pool> tp, bool cls,
      std::function<bool(std::string_view message)> checkForCompletionFunction,
      std::function<void(std::variant<std::string, error>)> callback);

  bool close();

private:
  enum class STARE {
    NECONECTAT,
    CONECTARE,
    CONECTAT,
    ASTEAPTA_MESAJ,
    TRIMITE_MESAJ,
    ASCULTA
  };

  int _sockfd; // socket file descriptor
  uint32_t _unique_id;
  STARE _stare;

  std::string _send_message;
  int _send_message_offset;
  std::string _receive_message;
  std::mutex _send_mutex;
  std::mutex _receive_mutex;

  void accepting_async_internal(
      std::shared_ptr<thread_pool> tp,
      std::function<void(std::variant<std::shared_ptr<tcp_connection>, error>)>
          callback);
};

} // namespace network::tcp
