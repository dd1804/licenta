#pragma once

#include <string>

namespace network {

struct url {
  bool is_valid;
  std::string scheme;
  std::string domain;
  std::string path;
  int portno;

  url(const std::string_view identifier);
};

} // namespace network
