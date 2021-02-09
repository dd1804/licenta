#include "url.h"

#include <cstdlib>

namespace network {

url::url(const std::string_view identifier) : portno(80), is_valid(false) {
  auto prot_pos = identifier.find("://");
  if (prot_pos == std::string::npos) {
    return;
  }

  is_valid = true;

  scheme = identifier.substr(0, prot_pos);
  auto port_pos = identifier.find(':', prot_pos + 3);
  if (port_pos != std::string::npos) {
    domain = identifier.substr(prot_pos + 3, port_pos - prot_pos - 3);
  }
  auto path_pos = identifier.find('/', prot_pos + 3);
  if (path_pos != std::string::npos) {
    if (port_pos == std::string::npos) {
      domain = identifier.substr(prot_pos + 3, path_pos - prot_pos - 3);
    } else {
      portno =
          atoi(identifier.substr(port_pos + 1, path_pos - port_pos - 1).data());
    }
    path = identifier.substr(path_pos);
  } else {
    if (port_pos == std::string::npos) {
      domain = identifier.substr(prot_pos + 3);
    } else {
      domain = identifier.substr(prot_pos + 3, port_pos);
    }
  }
  if (path == "") {
    path = "/";
  }
}

} // namespace network