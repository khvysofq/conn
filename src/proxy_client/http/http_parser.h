#ifndef PROXY_CLIENT_HTTP_PARSER_H_
#define PROXY_CLIENT_HTTP_PARSER_H_

#include <string>
#include <vector>
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/buffer.hpp>
#include "proxy_client/http/header.h"

namespace proxy{


  const std::string HTTP_CONNECT = "CONNECT";
  /// A Request received from a client.
  struct Request {
    /// The Request method, e.g. "GET", "POST".
    std::string method;

    /// The proxy requested host
    std::string host;

    /// The proxy port
    std::string port;

    /// The Requested URI, such as a path to a file.
    std::string uri;

    /// Major version number, usually 1.
    int http_version_major;

    /// Minor version number, usually 0 or 1.
    int http_version_minor;

    /// The headers included with the Request.
    std::vector<header> headers;

    /// The optional content sent with the Request.
    std::string content;

    /// Convert the Reply into a vector of buffers. The buffers do not own the
    /// underlying memory blocks, therefore the Reply object must remain valid and
    /// not be changed until the write operation has completed.
    std::vector<boost::asio::const_buffer> to_proxy_buffers();
  };

  struct FakeRequest{
    unsigned char key;
    unsigned char type;
    std::size_t content_length;
    std::string content;
  };
  struct FakeResult{
    int state_code;
    std::size_t content_length;
    std::string content;
    unsigned char key;
  };

  class HttpParser : boost::asio::coroutine{
  public:
    /// Parse some data. The tribool return value is true when a complete Request
    /// has been parsed, false if the data is invalid, indeterminate when more
    /// data is required. The InputIterator return value indicates how much of the
    /// input has been consumed.
    boost::tuple<boost::tribool, int> parse(Request& req,
      const char* data, int size)
    {
      int i = 0;
      for (; i < size; i++){
        boost::tribool result = consume(req, data[i]);
        if (result || !result)
          return boost::make_tuple(result, i);
      }
      boost::tribool result = boost::indeterminate;
      return boost::make_tuple(result, i);
    }

    boost::tuple<boost::tribool, int> ParserFakeResult(FakeResult& fake_res,
      const char* data, int size, char& key){
      int i = 0;
      for (; i < size; i++){
        boost::tribool result = ResultConsume(fake_res, data[i], key);
        if (result || !result)
          return boost::make_tuple(result, i);
      }
      boost::tribool result = boost::indeterminate;
      return boost::make_tuple(result, i);
    }

    boost::tuple<boost::tribool, int> ParserFakeRequest(FakeRequest& fake_req,
      const char* data, int size, char& key){
      int i = 0;
      for (; i < size; i++){
        boost::tribool result = RequestConsume(fake_req, data[i], key);
        if (result || !result)
          return boost::make_tuple(result, i);
      }
      boost::tribool result = boost::indeterminate;
      return boost::make_tuple(result, i);
    }
  private:

    /// Content length as decoded from headers. Defaults to 0.
    std::size_t content_length_;

    /// Handle the next character of input.
    boost::tribool consume(Request& req, char input);
    boost::tribool ResultConsume(FakeResult& fake_res, char c, char& key);
    std::vector<header> res_header_;

    boost::tribool RequestConsume(FakeRequest& fake_req, char c, char& key);

    /// Check if a byte is an HTTP character.
    static bool is_char(int c);

    /// Check if a byte is an HTTP control character.
    static bool is_ctl(int c);

    /// Check if a byte is defined as an HTTP tspecial character.
    static bool is_tspecial(int c);

    /// Check if a byte is a digit.
    static bool is_digit(int c);

    /// Check if two characters are equal, without regard to case.
    static bool tolower_compare(char a, char b);

    /// Check whether the two Request header names match.
    bool headers_equal(const std::string& a, const std::string& b);
  private:
  };
}

#endif // PROXY_CLIENT_HTTP_PARSER_H_