// Vision Zenith System Communication Protocol (Project)
#ifndef VZCONN_BASE_BASIC_DEFINES_H_
#define VZCONN_BASE_BASIC_DEFINES_H_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/signals2.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <iostream>
#include <string>
#include <iomanip>
#include <exception>

//TODO (Guangleihe) use boost basic type instead of this
#include "vzconn/base/basictypes.h"

namespace vzconn {

  typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
  typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;

  static const std::string DEFAULT_SERVER_ADDRESS = "";
  static const int DEFAULT_SERVER_PORT = 5298;
  //static const int DEFAULT_THREAD_COUNT = 2;
  static const int MAX_PACKET_SIZE = 1024 * 512; // 512 KB
  static const int FOREVER = -1;

  // The WriteHandler may be redefine with other type, a void * or a function
  // typedef void* WriteHandler;
  typedef void* WriteHandler;
  const WriteHandler WriteHandleDefaultValue = NULL;
}; // namespace vzconn

#endif // VZCONN_BASE_BASIC_DEFINES_H_