#include "tcp_connection.h"

#include "utils.h"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace network::tcp {

constexpr int MAX_LENGTH_PENDING_CONN_QUEUE = 5;
constexpr int READ_BUFFER_SIZE = 1024;

tcp_connection::tcp_connection()
    : _sockfd(-1), _unique_id(-1), _send_message_offset(0),
      _stare(STARE::NECONECTAT) {}

tcp_connection::tcp_connection(int sockfd, uint32_t unique_id)
    : _sockfd(sockfd), _unique_id(unique_id), _send_message_offset(0),
      _stare(STARE::CONECTAT) {}

tcp_connection::tcp_connection(const tcp_connection &other)
    : _sockfd(other._sockfd), _unique_id(other._unique_id),
      _send_message_offset(other._send_message_offset), _stare(other._stare) {}

bool tcp_connection::get_socket(bool non_blocking,
                                std::shared_ptr<thread_pool> tp) {
  if (_stare != STARE::NECONECTAT) {
    utils::err::log("get_socket: stare invalida");
    return false;
  }

  int opt = SOCK_STREAM;
  if (non_blocking)
    opt |= SOCK_NONBLOCK;
  _sockfd = ::socket(AF_INET, opt, IPPROTO_TCP);
  if (_sockfd < 0) {
    utils::err::log("get_socket: crearea socket-ului a esuat");
    return false;
  }

  if (non_blocking) {
    _unique_id = tp->get_id();
  }

  return true;
}

bool tcp_connection::connect(const url &serv_url) {
  if (!serv_url.is_valid || serv_url.scheme != "http") {
    utils::err::log("connect: url invalid");
    return false;
  }

  // ia adresa host-ului de la DNS
  struct hostent *server;
  server = ::gethostbyname(serv_url.domain.c_str());
  if (server == NULL) {
    utils::err::log("connect: nu exista host-ul");
    close();
    return false;
  }

  if (get_socket(false, nullptr) == false) {
    utils::err::log("connect: nu am primit un socket");
    close();
    return false;
  }

  struct sockaddr_in serv_addr;
  ::bzero(reinterpret_cast<char *>(&serv_addr), sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  ::bcopy(reinterpret_cast<char *>(server->h_addr),
          reinterpret_cast<char *>(&serv_addr.sin_addr.s_addr),
          server->h_length);
  serv_addr.sin_port = ::htons(serv_url.portno);
  if (::connect(_sockfd, reinterpret_cast<struct sockaddr *>(&serv_addr),
                sizeof(serv_addr)) < 0) {
    utils::err::log("connect: eroare conectare");
    close();
    return false;
  }

  _stare = STARE::CONECTAT;

  return true;
}

void tcp::tcp_connection::connect_async(const url &serv_url,
                                        std::shared_ptr<thread_pool> tp) {
  // utils::err::log("connect: " + std::to_string(_sockfd));
  if (_stare != STARE::NECONECTAT) {
    utils::err::log("connect_async: stare invalida");
    return;
  }

  if (!serv_url.is_valid || serv_url.scheme != "http") {
    utils::err::log("connect_async: url invalid");
    return;
  }

  auto instance = shared_from_this();

  struct hostent *server = NULL;
  int retries = 3;
  while (--retries > 0 && server == NULL) {
    server = ::gethostbyname(serv_url.domain.c_str());
  }
  if (server == NULL) {
    utils::err::log("connect_async: nu exista host-ul");
    close();
    return;
  }

  struct sockaddr_in serv_addr;
  ::bzero(reinterpret_cast<char *>(&serv_addr), sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  ::bcopy(reinterpret_cast<char *>(server->h_addr),
          reinterpret_cast<char *>(&serv_addr.sin_addr.s_addr),
          server->h_length);
  serv_addr.sin_port = ::htons(serv_url.portno);

  if (tp->add_fd(_sockfd, _unique_id) == false) {
    utils::err::log("connect_async: nu exista host-ul");
    close();
    return;
  }
  _stare = STARE::CONECTARE;
  auto r = ::connect(_sockfd, reinterpret_cast<sockaddr *>(&serv_addr),
                     sizeof(serv_addr));
  if (r < 0 && errno != EINPROGRESS) {
    utils::err::log("connect_async: eroare conectare");
    close();
    return;
  } else if (r == 0) {
    _stare = STARE::CONECTAT;
  }
}

bool tcp_connection::listen(const int portno, bool non_blocking) {
  if (_stare != STARE::NECONECTAT) {
    utils::err::log("listen: stare invalida");
    return false;
  }

  struct sockaddr_in serv_addr;
  int opt = SOCK_STREAM;
  if (non_blocking)
    opt |= SOCK_NONBLOCK;
  _sockfd = ::socket(AF_INET, opt, IPPROTO_TCP);
  if (_sockfd < 0) {
    utils::err::log("listen: crearea socket-ului a esuat");
    return false;
  }

  bzero(reinterpret_cast<char *>(&serv_addr), sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (::bind(_sockfd, reinterpret_cast<struct sockaddr *>(&serv_addr),
             sizeof(serv_addr)) < 0) {
    utils::err::log("listen: bind-ul socket-ului la port a esuat");
    close();
    return false;
  }

  if (::listen(_sockfd, MAX_LENGTH_PENDING_CONN_QUEUE) < 0) {
    utils::err::log("listen: asteptarea unei conexiuni a esuat a esuat");
    close();
    return false;
  }

  _stare = STARE::ASCULTA;

  return true;
}

bool tcp::tcp_connection::listen_async(
    const int portno, std::shared_ptr<thread_pool> tp,
    std::function<void(std::variant<std::shared_ptr<tcp_connection>, error>)>
        callback) {

  if (listen(portno, true) == false) {
    return false;
  }

  _unique_id = tp->get_id();

  auto instance = shared_from_this();
  tp->add_callback(_unique_id, thread_pool::type::IN,
                   [instance, tp, callback](bool success) {
                     if (success) {
                       instance->accepting_async_internal(tp, callback);
                     } else {
                       utils::err::log("accept_async: erorare conexiune");
                     }
                     return true;
                   },
                   true);

  if (tp->add_fd(_sockfd, _unique_id) == false) {
    utils::err::log("accept_async: eroare epoll");
    close();
    return false;
  }

  return true;
}

std::variant<tcp_connection, tcp_connection::error> tcp_connection::accept() {
  if (_stare != STARE::ASCULTA) {
    utils::err::log("accept: stare invalida");
    return std::variant<tcp_connection, tcp_connection::error>(error());
  }

  int newsockfd, clilen;
  struct sockaddr_in cli_addr;

  clilen = sizeof(cli_addr);
  newsockfd = ::accept(_sockfd, reinterpret_cast<struct sockaddr *>(&cli_addr),
                       reinterpret_cast<socklen_t *>(&clilen));

  if (newsockfd < 0) {
    utils::err::log("accept: stare invalida");
    return std::variant<tcp_connection, tcp_connection::error>(error());
  }

  return std::variant<tcp_connection, tcp_connection::error>(
      tcp_connection(newsockfd, -1));
}

void tcp_connection::accepting_async_internal(
    std::shared_ptr<thread_pool> tp,
    std::function<void(std::variant<std::shared_ptr<tcp_connection>, error>)>
        callback) {

  int newsockfd, clilen;
  struct sockaddr_in cli_addr;

  clilen = sizeof(cli_addr);
  newsockfd = ::accept(_sockfd, reinterpret_cast<struct sockaddr *>(&cli_addr),
                       reinterpret_cast<socklen_t *>(&clilen));

  if (newsockfd < 0) {
    utils::err::log("accept_async_internal: eroare conectare");
    callback(std::variant<std::shared_ptr<tcp_connection>, error>(error()));
    return;
  }

  int flags, ret;
  flags = ::fcntl(newsockfd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl");
    return;
  }

  flags |= O_NONBLOCK;
  ret = ::fcntl(newsockfd, F_SETFL, flags);
  if (ret == -1) {
    perror("fcntl");
    return;
  }

  auto unique_id = tp->get_id();

  callback(std::variant<std::shared_ptr<tcp_connection>, tcp_connection::error>(
      std::make_shared<tcp_connection>(newsockfd, unique_id)));

  if (tp->add_fd(newsockfd, unique_id) == false) {
    utils::err::log("accect_async: eroare epoll");
    close();
    return;
  }
}

bool tcp_connection::send(const std::string_view message) {
  _stare = STARE::TRIMITE_MESAJ;

  auto size = ::write(_sockfd, message.data(), message.size());
  if (size < 0) {
    utils::err::log("send: trimiterea a esuat");
    close();
    return false;
  }
  if (size != message.size()) {
    utils::err::log("send: mesaj trimis partial");
    close();
    return false;
  }

  _stare = STARE::CONECTAT;

  return true;
}

void tcp_connection::set_send_callback(std::string message,
                                       std::shared_ptr<thread_pool> tp,
                                       bool cls,
                                       std::function<void(bool)> callback) {
  _send_message = message;
  _send_message_offset = 0;
  auto instance = shared_from_this();

  auto cb = [this, instance, tp, cls, callback](bool success) mutable {
    bool callCallback = false;
    bool isError = false;
    {
      std::lock_guard lock(_send_mutex);

      if (_stare == STARE::NECONECTAT) {
        utils::debug::log(
            "set_send_callback: stare invalida " + std::to_string((int)_stare) +
            " " + std::to_string(_sockfd) + " " + std::to_string(_unique_id));
        return false;
      }
      if (!success) {
        utils::err::log("set_send_callback: trimiterea a esuat");
        callCallback = true;
        isError = true;
      }

      if (!isError) {
        for (;;) {
          _stare = STARE::TRIMITE_MESAJ;

          auto size =
              ::write(_sockfd, _send_message.c_str() + _send_message_offset,
                      _send_message.size() - _send_message_offset);

          if (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            utils::err::log("set_send_callback: trimiterea a esuat");
            callCallback = true;
            isError = true;
            break;
          } else if (size >= _send_message.size() - _send_message_offset) {
            callCallback = true;
            break;
          }

          _send_message_offset += size;

          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
          }
        }
      }

      if (callCallback) {
        _send_message.clear();
        _send_message_offset = 0;

        if (!isError) {
          if (cls) {
            close();
          } else {
            _stare = STARE::CONECTAT;
          }
        } else {
          close();
        }
      }
    }

    if (callCallback) {
      if (!isError) {
        callback(true);
      } else {
        callback(false);
      }
      return true;
    }

    return false;
  };

  tp->add_callback(_unique_id, thread_pool::type::OUT, cb);
}

std::variant<std::string, tcp_connection::error> tcp_connection::receive() {
  if (_stare != STARE::CONECTAT) {
    utils::debug::log("receive: stare invalida");
    return std::variant<std::string, tcp_connection::error>(error());
  }

  int n = 0;
  char buffer[READ_BUFFER_SIZE];
  std::string response = "";

  _stare = STARE::ASTEAPTA_MESAJ;

  while (true) {
    ::bzero(buffer, READ_BUFFER_SIZE);
    n = ::read(_sockfd, buffer, READ_BUFFER_SIZE);
    if (n < 0) {
      utils::debug::log("receive: primirea a esuat");
      close();
      return std::variant<std::string, tcp_connection::error>(error());
    }
    response += std::string(buffer);
    if (n != READ_BUFFER_SIZE) {
      break;
    }
  }

  _stare = STARE::CONECTAT;

  return std::variant<std::string, tcp_connection::error>(response);
}

void tcp_connection::set_receive_callback(
    std::shared_ptr<thread_pool> tp, bool cls,
    std::function<bool(std::string_view message)> checkForCompletionFunction,
    std::function<void(std::variant<std::string, error>)> callback) {

  auto instance = shared_from_this();
  tp->add_callback(
      _unique_id, thread_pool::type::IN,
      [this, instance, tp, cls, checkForCompletionFunction,
       callback](bool success) mutable {
        bool callCallback = false;
        bool isError = false;
        std::string message;
        {
          std::lock_guard lock(_receive_mutex);

          if (_stare == STARE::NECONECTAT) {
            utils::debug::log("set_receive_callback: stare invalida " +
                              std::to_string((int)_stare));
            return false;
          }
          if (!success) {
            utils::err::log("set_receive_callback: primirea a esuat");
            callCallback = true;
            isError = true;
          }

          if (!isError) {
            _stare = STARE::ASTEAPTA_MESAJ;
            for (;;) {
              char buffer[READ_BUFFER_SIZE];

              auto size = ::read(_sockfd, buffer, READ_BUFFER_SIZE);
              if (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                utils::err::log(
                    "set_receive_callback: primirea a esuat " +
                    std::to_string(errno) + std::to_string((int)_stare) + " " +
                    std::to_string(_sockfd) + " " + std::to_string(_unique_id));
                callCallback = true;
                isError = true;
                break;
              } else if (size == 0) {
                callCallback = true;
                break;
              }

              if (size < READ_BUFFER_SIZE) {
                buffer[size] = 0;
              }
              _receive_message += buffer;

              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
              }
            }

            if (checkForCompletionFunction(_receive_message)) {
              callCallback = true;
            }

            if (callCallback) {
              message = _receive_message;
              _receive_message.clear();

              if (!isError) {
                if (cls) {
                  close();
                } else {
                  _stare = STARE::CONECTAT;
                }
              } else {
                close();
              }
            }
          }
        }

        if (callCallback) {
          if (!isError) {
            callback(std::variant<std::string, tcp_connection::error>(
                std::move(message)));
          } else {
            callback(std::variant<std::string, tcp_connection::error>(error()));
          }
          return true;
        }

        return false;
      });
}

bool tcp_connection::close() {
  bool success = true;
  if (::close(_sockfd) < 0) {
    utils::err::log("close: inchiderea conexiunii tcp a esuat");
    success = false;
  }
  _stare = STARE::NECONECTAT;
  return success;
}

} // namespace network::tcp
