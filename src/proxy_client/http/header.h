
#ifndef PROXY_CLIENT_HEADER_HPP
#define PROXY_CLIENT_HEADER_HPP

#include <string>

namespace proxy {

  struct header{
    std::string name;
    std::string value;
  };

  const std::string CONTENT_LENTH = "Content-Length";
  const std::string PROXY_CONNECTION = "Proxy-Connection";
  const std::string CONNECTION = "Connection";
  const char SEPARATOR[] = { ':', ' ' };
  const char CRLF[] = { '\r', '\n' };
  const char SPACE[] = { ' ' };
  const char HTTP_1_1[] = { ' ', 'H', 'T', 'T', 'P', '/', '1', '.', '1' };
  const char CHA_KEY = 'r';
  const char CHA_TYPE = 'a';
  const char CHA_EQ = '=';
  const char CHAR_TYPE_CONNECT = 'e';
  const char CHAR_TYPE_DATA = '3';


} // namespace proxy

#endif // PROXY_CLIENT_HEADER_HPP
