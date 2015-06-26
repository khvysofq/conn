#include "proxy_client/fake/fakeparser.h"

namespace proxy{

  void ConstBufferToBytebufferD(
    const std::vector<boost::asio::const_buffer>& const_buffer,
    vzconn::ByteBuffer * byte_buffer, unsigned char key){
    for (std::size_t i = 0; i < const_buffer.size(); i++){
      const char* data = boost::asio::buffer_cast<const char*>(const_buffer[i]);
      std::size_t data_size = boost::asio::buffer_size(const_buffer[i]);
      ConstCharToBytebufferD(data, data_size, byte_buffer, key);
    }
  }

  void ConstCharToBytebufferD(const char* data, int data_size,
    vzconn::ByteBuffer * byte_buffer, unsigned char key){
    for (int j = 0; j < data_size; j++){
      unsigned char t = data[j];
      t = (t >> 4) | (t << 4);
      byte_buffer->WriteUInt8(t ^ key);
    }
  }
  void ConstCharToBytebufferE(const char* data, int data_size,
    vzconn::ByteBuffer * byte_buffer, unsigned char key){
    for (int j = 0; j < data_size; j++){
      unsigned char t = data[j] ^ key;
      t = (t >> 4) | (t << 4);
      byte_buffer->WriteUInt8(t);
    }
  }
}