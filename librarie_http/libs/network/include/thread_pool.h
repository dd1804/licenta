#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <set>
#include <thread>
#include <vector>

namespace network {

class thread_pool : public std::enable_shared_from_this<thread_pool> {
public:
  enum type { IN = 1, OUT = 2 };

  thread_pool(const size_t, bool test);

  void init();

  void run();

  void stop();

  uint32_t get_id();

  bool add_fd(int fd, int id);

  void add_callback(int id, int tp, std::function<bool(bool)> callback,
                    bool perm = false);

  void remove_callback(int id, int tp);

  void run_on_thread(std::function<void()> callback);

  ~thread_pool();

private:
  std::vector<std::thread> _internal_threads;

  int _epollfd;
  int _interrupt_eventfd;
  int _run_on_thread_eventfd;
  int _size;
  std::atomic<bool> _running;
  std::map<int, std::function<bool(bool)>> _in_callbacks;
  std::map<int, std::function<bool(bool)>> _out_callbacks;
  std::set<int> _perm;
  std::queue<std::function<void()>> _run_on_thread_callbacks;
  std::mutex _in_mutex;
  std::mutex _out_mutex;
  std::mutex _run_on_thread_mutex;
};

} // namespace network
