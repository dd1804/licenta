#include <atomic>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "client.h"
#include "log.h"
#include "server.h"
#include "thread_pool.h"

void test_client(const std::string server) {
  network::http::client http_client;

  std::unordered_map<std::string, std::string> headers;
  headers["Authorization"] = "123456";

  network::http::message_body message_body;
  message_body.content_type = "application/json";
  message_body.message = "{\"test\": 1 }";

  auto response = http_client.post(server, headers, message_body);
  std::cout << "Raspunsul serverului:" << std::endl;
  std::cout << response.to_string();
}

void test_client_async(const std::string server, int numar_threaduri,
                       int numar_requesturi) {
  auto thread_pool =
      std::make_shared<network::thread_pool>(numar_threaduri, false);
  thread_pool->init();

  std::atomic<int> count(0);
  std::atomic<int> successfull(0);
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < numar_requesturi; ++i) {
    network::http::client http_client(thread_pool);

    std::unordered_map<std::string, std::string> headers = {
        {"Accept", "*"}, {"Accept-Encoding", ""}};
    network::http::message_body message_body;

    message_body.content_type = "application/json";
    message_body.message = "{\"camp1\": \"valoare1\", \"camp2\": \"valoare2\"}";

    http_client.post_async(
        server, headers, std::move(message_body),
        [&start, &count, &numar_requesturi,
         &successfull](network::http::http_response response) {
          std::cout << "Am primit raspunsul:\n"
                    << response.to_string() << std::endl;

          if (response.status_code == 200) {
            ++successfull;
          }
          if (++count == numar_requesturi) {
            std::cout << "Numar de requesturi cu status code 200: "
                      << successfull << std::endl;
            auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Timp de rulare in microsecunde: "
                      << std::chrono::duration_cast<std::chrono::microseconds>(
                             end - start)
                             .count()
                      << std::endl;
            exit(0);
          }
        });
  }

  std::this_thread::sleep_for(std::chrono::seconds(100));
}

void test_server(int portno) {
  network::http::server server;
  if (server.listen(portno) == false) {
    return;
  }

  std::cout << "serverul a pornit pe portul " << portno << " ..." << std::endl;
  auto request = server.accept();
  // afiseaza requestul primit
  std::cout << request.get_http_request().to_string();
  // raspunde
  if (request.get_http_request().method == network::http::method::POST) {

    if (request.get_http_request().uri == "/resurse/1") {
      request.respond(network::http::http_response(
          network::http::status::OK,
          {"application/json", "{ \"reprezentare\": 1 }"}));
    } else {
      request.respond(
          network::http::http_response(network::http::status::NOT_FOUND));
    }

  } else {
    request.respond(
        network::http::http_response(network::http::status::NOT_IMPLEMENTED));
  }
}

void test_server_async(int portno, int numar_threaduri) {
  auto thread_pool =
      std::make_shared<network::thread_pool>(numar_threaduri, false);
  thread_pool->init();
  network::http::server server(thread_pool);
  auto success = server.listen_async(
      portno, [](std::unique_ptr<network::http::server::request> request) {
        // afiseaza requestul primit
        std::cout << "Am primit requestul:\n"
                  << request->get_http_request().to_string() << "\n\n";
        // raspunde
        if (request->get_http_request().method == network::http::method::POST) {

          if (request->get_http_request().uri == "/resurse/1") {
            request->respond_async(network::http::http_response(
                network::http::status::OK,
                {"application/json", "{ \"reprezentare\": 1 }"}));
          } else {
            request->respond_async(
                network::http::http_response(network::http::status::NOT_FOUND));
          }

        } else {
          std::cout << "Am primit requestul:\n"
                    << request->get_http_request().to_string() << "\n\n";
          request->respond_async(network::http::http_response(
              network::http::status::NOT_IMPLEMENTED));
        }
      });

  if (success) {
    std::cout << "serverul a pornit pe portul " << portno << " ..."
              << std::endl;
  } else {
    std::cout << "portul " << portno << " este ocupat" << std::endl;
  }

  std::this_thread::sleep_for(std::chrono::seconds(1000));
}

int main() {
  int numar_threaduri = 4;
  int port = 45600; // server librarie
  // int port = 54300; // server python
  std::string server =
      "http://localhost:" + std::to_string(port) + "/resurse/1";
  // std::string server = "http://httpbin.org/post"; // server online

  // test_client(server);

  int numar_requesturi = 10;
  test_client_async(server, numar_threaduri, numar_requesturi);

  // test_server(port);

  // test_server_async(port, numar_threaduri);

  return 0;
}
