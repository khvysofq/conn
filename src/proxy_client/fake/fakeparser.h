#ifndef PROXY_CLIENT_FAKE_PARSER_H_
#define PROXY_CLIENT_FAKE_PARSER_H_

#include "proxy_client/fake/fakedefine.h"

namespace proxy{

  void ConstBufferToBytebufferD(const std::vector<boost::asio::const_buffer>& const_buffer,
    vzconn::ByteBuffer * byte_buffer, unsigned char key);
  void ConstCharToBytebufferD(const char* data, int data_size,
    vzconn::ByteBuffer * byte_buffer, unsigned char key);
  void ConstCharToBytebufferE(const char* data, int data_size,
    vzconn::ByteBuffer * byte_buffer, unsigned char key);
}

#endif // PROXY_CLIENT_FAKE_PARSER_H_