#ifndef PROXY_CLIENT_FAKE_DEFINE_H_
#define PROXY_CLIENT_FAKE_DEFINE_H_

#include "proxy_client/http/header.h"
#include "proxy_client/base/proxybase.h"

namespace proxy{
  const std::string FAKE_HEADER_REQUEST_START = "POST";
  const std::string FAKE_HEADER_REQUES_END = ".asp";
  const std::string FAKE_HEADER_REQUEST_END = "HTTP/1.1\r\n";
  const std::string FAKE_CLIENT_HEADER = "Accept: */*\r\n"
    "User-Agent: User-Agent	Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n"
    "Host: www.bing.com\r\n"
    "Content-Type: application/x-active\r\n"
    "Connection: Keep-Alive\r\n";
  const std::string FAKE_CONTENT_LENGTH = "Content-Length: ";
  const char SLASH = '/';
  const int MAX_KEY_RAND_SIZE = 26;
  const char RAND_START_CHAR = 'a';
  const char CRCFCRCF[] = {'\r','\n','\r','\n'};
  const int CRCFCRCF_SIZE = 4;

  // The proxy client settings
  const int STATE_CONNECT_SUCCEED = 200;
  const int STATE_CONNECT_FAILURE = 400;

  const std::string FAKE_RESULT_200 = "HTTP/1.1 200 OK\r\n"
    "Cache-Control: private, max-age=0\r\n"
    "Connection: keep-live\r\n"
    "Content-Type: application/x-active\r\n"
    "Vary: Accept-Encoding\r\n"
    "Server: Microsoft-IIS/8.5\r\n\r\n";

  const std::string FAKE_RESULT_400 = "HTTP/1.1 400 Bad Request\r\n"
    "Cache-Control: private, max-age=0\r\n"
    "Connection: Keep-Alive\r\n\r\n";
}

#endif // PROXY_CLIENT_FAKE_DEFINE_H_