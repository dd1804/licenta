#include "thread_pool.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr int MAX_EVENTS = 64;

namespace network {

thread_pool::~thread_pool() {
  ::close(_interrupt_eventfd);
  ::close(_run_on_thread_eventfd);
  ::close(_epollfd);
}

thread_pool::thread_pool(const size_t size, bool test)
    : _epollfd(-1), _size(size), _running(false) {
  _epollfd = ::epoll_create1(0);
  if (_epollfd < 0) {
    utils::err::log("nu am putut crea epoll file descriptor");
    abort();
  }
}

void thread_pool::init() {
  _running = true;
  for (size_t i = 0; i < _size; ++i) {
    _internal_threads.emplace_back([this, i]() {
      std::shared_ptr<thread_pool> instance = shared_from_this();
      run();
    });
  }
}

void thread_pool::run() {
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));
  struct epoll_event *events =
      reinterpret_cast<epoll_event *>(::calloc(MAX_EVENTS, sizeof(event)));

  while (_running.load(std::memory_order_consume)) {

    int nevents = 0;
    nevents = epoll_wait(_epollfd, events, MAX_EVENTS, -1);
    // utils::err::log("wait ...");
    //} while (nevents < 0 && errno == EINTR);
    if (nevents < 0) {
      if (errno != EINTR) {
        utils::err::log("eroare epoll_wait ");
      }
      continue;
    }

    for (int i = 0; i < nevents; ++i) {

      if (events[i].data.fd == _interrupt_eventfd) {

        eventfd_t val;
        eventfd_read(_interrupt_eventfd, &val);

      } else if (events[i].data.fd == _run_on_thread_eventfd) {

        eventfd_t val;
        eventfd_read(_run_on_thread_eventfd, &val);
        std::function<void()> callback;
        {
          std::lock_guard guard(_run_on_thread_mutex);
          callback = _run_on_thread_callbacks.front();
          _run_on_thread_callbacks.pop();
        }
        callback();

      } else {

        bool eroare = false;
        int result;
        socklen_t result_len = sizeof(result);
        if (events[i].events & EPOLLERR) {
          eroare = true;
        } else if (events[i].events & EPOLLHUP && events[i].events & EPOLLOUT) {
          continue;
        }
        if (events[i].events & EPOLLIN) {
          std::function<bool(bool)> in_callbk;
          {
            std::lock_guard guard(_in_mutex);
            auto callbackIt = _in_callbacks.find(events[i].data.u32);
            if (callbackIt != _in_callbacks.end()) {
              in_callbk = callbackIt->second;
            }
          }
          if (in_callbk && in_callbk(!eroare)) {
            std::lock_guard guard(_in_mutex);
            if (_perm.find(events[i].data.u32) == _perm.end())
              _in_callbacks.erase(events[i].data.u32);
          }
        }
        if (events[i].events & EPOLLOUT) {
          std::function<bool(bool)> out_callbk;
          {
            std::lock_guard guard(_out_mutex);
            auto callbackIt = _out_callbacks.find(events[i].data.u32);
            if (callbackIt != _out_callbacks.end()) {
              out_callbk = callbackIt->second;
            }
          }
          if (out_callbk && out_callbk(!eroare)) {
            std::lock_guard guard(_out_mutex);
            _out_callbacks.erase(events[i].data.u32);
          }
        }
      }
    }
  }

  delete events;
  utils::debug::log("iesire thread");
}

void thread_pool::stop() {
  _running.store(false, std::memory_order_acquire);
  if (eventfd_write(_interrupt_eventfd, _size) < 0) {
    utils::err::log("stop: scrierea eventului de intrerupere a esuat");
  }
  utils::debug::log("~thread_pool() joining ...");
  for (auto &thread : _internal_threads) {
    if (thread.joinable()) {
      thread.join();
    } else {
      utils::err::log("thread-ul nu e joinable");
    }
  }
  utils::debug::log("~thread_pool() joined");
}

uint32_t thread_pool::get_id() {
  static std::atomic<uint32_t> unique_id = 0;
  auto id = ++unique_id;
  while (id == _interrupt_eventfd || id == _epollfd) {
    id = ++unique_id;
  }
  return id;
}

bool thread_pool::add_fd(int fd, int id) {
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLOUT | EPOLLET;
  event.data.u32 = id;
  if (::epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    utils::err::log("add_fd: epoll_ctl a esuat");
    return false;
  }
  return true;
  ;
}

void thread_pool::add_callback(int id, int tp,
                               std::function<bool(bool)> callback, bool perm) {
  if (tp & type::IN) {
    std::lock_guard guard(_in_mutex);
    _in_callbacks[id] = callback;
    if (perm)
      _perm.insert(id);
  } else if (tp & type::OUT) {
    std::lock_guard guard(_out_mutex);
    _out_callbacks[id] = callback;
  }
}

void thread_pool::remove_callback(int id, int tp) {
  if (tp & type::IN) {
    std::lock_guard guard(_in_mutex);
    _in_callbacks.erase(id);
  }
  if (tp & type::OUT) {
    std::lock_guard guard(_out_mutex);
    _out_callbacks.erase(id);
  }
}

void thread_pool::run_on_thread(std::function<void()> callback) {
  {
    std::lock_guard guard(_run_on_thread_mutex);
    _run_on_thread_callbacks.push(callback);
  }
  if (eventfd_write(_run_on_thread_eventfd, 1) < 0) {
    utils::err::log("run_on_thread: scrierea eventului a esuat");
  }
}

} // namespace network
