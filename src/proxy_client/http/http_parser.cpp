#include "proxy_client/http/http_parser.h"
#include "proxy_client/base/proxybase.h"
#include <algorithm>
#include <cctype>
#include <boost/lexical_cast.hpp>
#include "proxy_client/fake/fakedefine.h"

namespace proxy{


  std::vector<boost::asio::const_buffer> Request::to_proxy_buffers(){

    std::vector<boost::asio::const_buffer> buffers;

    buffers.push_back(boost::asio::buffer(method));
    buffers.push_back(boost::asio::buffer(SPACE));
    buffers.push_back(boost::asio::buffer(uri));
    buffers.push_back(boost::asio::buffer(HTTP_1_1));
    buffers.push_back(boost::asio::buffer(CRLF));

    for (std::size_t i = 0; i < headers.size(); ++i)
    {
      header& h = headers[i];
      buffers.push_back(boost::asio::buffer(h.name));
      buffers.push_back(boost::asio::buffer(SEPARATOR));
      buffers.push_back(boost::asio::buffer(h.value));
      buffers.push_back(boost::asio::buffer(CRLF));
    }
    buffers.push_back(boost::asio::buffer(CRLF));
    buffers.push_back(boost::asio::buffer(content));
    return buffers;
  }

  // Enable the pseudo-keywords reenter, yield and fork.
#include <boost/asio/yield.hpp>

  boost::tribool HttpParser::consume(Request& req, char c)
  {
    reenter(this)
    {
      req.method.clear();
      req.uri.clear();
      req.port.clear();
      req.http_version_major = 0;
      req.http_version_minor = 0;
      req.headers.clear();
      req.content.clear();
      content_length_ = 0;

      // Request method.
      while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ' '){
        req.method.push_back(c);
        yield return boost::indeterminate;
      }
      if (req.method.empty())
        return false;

      // Space.
      if (c != ' '){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      if (req.method != HTTP_CONNECT){
        // http://
        if (c != 'h'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != 't'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != 't'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != 'p'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != ':'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != '/'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != '/'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
      }

      // host
      while (!is_ctl(c) && c != ' ' && c != '/'){
        if (c == ':'){
          yield return boost::indeterminate;
          break;
        }
        req.host.push_back(c);
        yield return boost::indeterminate;
      }
      if (req.host.empty()){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      
      // Read the port
      while (!is_ctl(c) && c != ' ' && c != '/' && is_digit(c)){
        req.port.push_back(c);
        yield return boost::indeterminate;
      }
      if (req.port.empty()){
        if (req.method == HTTP_CONNECT){
          req.port = "443";
        }
        else{
          req.port = "80";
        }
      }

      // URI.
      while (!is_ctl(c) && c != ' '){
        req.uri.push_back(c);
        yield return boost::indeterminate;
      }
      if (req.uri.empty() && req.method != HTTP_CONNECT) return false;

      // Space.
      if (c != ' '){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // HTTP protocol identifier.
      if (c != 'H'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != 'T'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != 'T'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != 'P'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // Slash.
      if (c != '/'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // Major version number.
      if (!is_digit(c)){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      while (is_digit(c)){
        req.http_version_major = req.http_version_major * 10 + c - '0';
        yield return boost::indeterminate;
      }

      // Dot.
      if (c != '.'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // Minor version number.
      if (!is_digit(c)){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      while (is_digit(c)){
        req.http_version_minor = req.http_version_minor * 10 + c - '0';
        yield return boost::indeterminate;
      }

      // CRLF.
      if (c != '\r'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '\n'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // Headers.
      while ((is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != '\r')
        || (c == ' ' || c == '\t')){
        if (c == ' ' || c == '\t'){
          // Leading whitespace. Must be continuation of previous header's value.
          if (req.headers.empty()){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          while (c == ' ' || c == '\t')
            yield return boost::indeterminate;
        }
        else{
          // Start the next header.
          req.headers.push_back(header());

          // Header name.
          while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ':'){
            req.headers.back().name.push_back(c);
            yield return boost::indeterminate;
          }

          // Colon and space separates the header name from the header value.
          if (c != ':'){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          yield return boost::indeterminate;
          if (c != ' '){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          yield return boost::indeterminate;
        }

        // Header value.
        while (c != '\r'){
          req.headers.back().value.push_back(c);
          yield return boost::indeterminate;
        }

        // CRLF.
        if (c != '\r'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != '\n'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
      }

      // CRLF.
      if (c != '\r'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '\n'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }

      // Check for optional Content-Length header.
      for (std::size_t i = 0; i < req.headers.size(); ++i){
        if (headers_equal(req.headers[i].name, CONTENT_LENTH)){
          try{
            content_length_ =
              boost::lexical_cast<std::size_t>(req.headers[i].value);
          }
          catch (boost::bad_lexical_cast& e){
            std::cout << "Parser error " << e.what() << std::endl;
            // DLOG(ERROR) << "Parser error " << e.what();
            return false;
          }
        }
        else if (headers_equal(req.headers[i].name, PROXY_CONNECTION)){
          req.headers[i].name = CONNECTION;
          req.headers[i].value = "close";
        }
        else if (headers_equal(req.headers[i].name, CONNECTION)){
          req.headers[i].value = "close";
        }
      }

      // Content.
      while (req.content.size() < content_length_){
        yield return boost::indeterminate;
        req.content.push_back(c);
      }
    }
    return true;
  }
  // Disable the pseudo-keywords reenter, yield and fork.

  boost::tribool HttpParser::ResultConsume(FakeResult& fake_res, char c, char& key){

    reenter(this){
      fake_res.content.clear();
      fake_res.content_length = 0;
      fake_res.state_code = 0;
      // HTTP protocol identifier.
      if (c != 'H'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != 'T'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != 'T'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != 'P'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '/'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '1'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '.'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '1'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != ' '){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      
      // Read the state code
      while (is_digit(c)){
        fake_res.state_code = fake_res.state_code * 10 + (c - '0');
        yield return boost::indeterminate;
      }

      if (c != ' '){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      while (c != '\r'){
        yield return boost::indeterminate;
      }

      // CRLF.
      if (c != '\r'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '\n'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // Headers.
      while ((is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != '\r')
        || (c == ' ' || c == '\t')){
        if (c == ' ' || c == '\t'){
          // Leading whitespace. Must be continuation of previous header's value.
          if (res_header_.empty()){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          while (c == ' ' || c == '\t')
            yield return boost::indeterminate;
        }
        else{
          // Start the next header.
          res_header_.push_back(header());

          // Header name.
          while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ':'){
            res_header_.back().name.push_back(c);
            yield return boost::indeterminate;
          }

          // Colon and space separates the header name from the header value.
          if (c != ':'){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          yield return boost::indeterminate;
          if (c != ' '){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          yield return boost::indeterminate;
        }

        // Header value.
        while (c != '\r'){
          res_header_.back().value.push_back(c);
          yield return boost::indeterminate;
        }

        // CRLF.
        if (c != '\r'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != '\n'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
      }

      // Check for optional Content-Length header.
      for (std::size_t i = 0; i < res_header_.size(); ++i){
        if (headers_equal(res_header_[i].name, CONTENT_LENTH)){
          try{
            fake_res.content_length =
              boost::lexical_cast<std::size_t>(res_header_[i].value);
          }
          catch (boost::bad_lexical_cast& e){
            std::cout << "Parser error " << e.what() << std::endl;
            // DLOG(ERROR) << "Parser error " << e.what();
            return false;
          }
        }
      }
      // CRLF.
      if (c != '\r'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '\n'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }

      // Content.
      while (fake_res.content.size() < fake_res.content_length){
        yield return boost::indeterminate;
        unsigned char t = c;
        c = c ^ key;
        key = t;
        fake_res.content.push_back(c);
      }
      fake_res.key = key;
    }
    return true;
  }

  boost::tribool HttpParser::RequestConsume(FakeRequest& fake_req, char c, char& key){

    reenter(this)
    {
      fake_req.content.clear();
      fake_req.content_length = 0;
      fake_req.key = 0;
      fake_req.type = 0;
      res_header_.clear();

      // Request method.
      while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ' '){
        yield return boost::indeterminate;
      }
      // Space.
      if (c != ' '){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      if (c != SLASH){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      if (c != CHA_KEY){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      if (is_ctl(c) || c == ' ' || c == '/'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      fake_req.key = c;
      key = c;
      yield return boost::indeterminate;

      if (c != CHA_TYPE){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      if (is_ctl(c) || c == ' ' || c == '/'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      fake_req.type = c;
      yield return boost::indeterminate;


      // host
      while (c != '\r'){
        yield return boost::indeterminate;
      }
      // CRLF.
      if (c != '\r'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '\n'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;

      // Headers.
      while ((is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != '\r')
        || (c == ' ' || c == '\t')){
        if (c == ' ' || c == '\t'){
          // Leading whitespace. Must be continuation of previous header's value.
          if (res_header_.empty()){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          while (c == ' ' || c == '\t')
            yield return boost::indeterminate;
        }
        else{
          // Start the next header.
          res_header_.push_back(header());

          // Header name.
          while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ':'){
            res_header_.back().name.push_back(c);
            yield return boost::indeterminate;
          }

          // Colon and space separates the header name from the header value.
          if (c != ':'){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          yield return boost::indeterminate;
          if (c != ' '){
            // DLOG(ERROR) << "Parser error";
            return false;
          }
          yield return boost::indeterminate;
        }

        // Header value.
        while (c != '\r'){
          res_header_.back().value.push_back(c);
          yield return boost::indeterminate;
        }

        // CRLF.
        if (c != '\r'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
        if (c != '\n'){
          // DLOG(ERROR) << "Parser error";
          return false;
        }
        yield return boost::indeterminate;
      }

      // CRLF.
      if (c != '\r'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }
      yield return boost::indeterminate;
      if (c != '\n'){
        // DLOG(ERROR) << "Parser error";
        return false;
      }

      // Check for optional Content-Length header.
      for (std::size_t i = 0; i < res_header_.size(); ++i){
        if (headers_equal(res_header_[i].name, CONTENT_LENTH)){
          try{
            fake_req.content_length =
              boost::lexical_cast<std::size_t>(res_header_[i].value);
          }
          catch (boost::bad_lexical_cast& e){
            std::cout << "Parser error " << e.what() << std::endl;
            // DLOG(ERROR) << "Parser error " << e.what();
            return false;
          }
        }
      }

      // Content.
      while (fake_req.content.size() < fake_req.content_length){
        yield return boost::indeterminate;
        unsigned char t = c;
        c = c ^ key;
        key = t;
        fake_req.content.push_back(c);
      }
      key = fake_req.key;
    }
    return true;
  }

#include <boost/asio/unyield.hpp>

  bool HttpParser::is_char(int c)
  {
    return c >= 0 && c <= 127;
  }

  bool HttpParser::is_ctl(int c)
  {
    return (c >= 0 && c <= 31) || (c == 127);
  }

  bool HttpParser::is_tspecial(int c)
  {
    switch (c)
    {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}': case ' ': case '\t':
      return true;
    default:
      return false;
    }
  }

  bool HttpParser::is_digit(int c)
  {
    return c >= '0' && c <= '9';
  }

  bool HttpParser::tolower_compare(char a, char b)
  {
    return std::tolower(a) == std::tolower(b);
  }

  bool HttpParser::headers_equal(const std::string& a, const std::string& b)
  {
    if (a.length() != b.length())
      return false;

    return std::equal(a.begin(), a.end(), b.begin(),
      &HttpParser::tolower_compare);
  }

}