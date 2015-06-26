#ifndef DATABASE_SERVER_DATABASE_SESSION_H_
#define DATABASE_SERVER_DATABASE_SESSION_H_

#include "boost/noncopyable.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/asio.hpp"
#include "mysql_driver.h"
#include "vzconn/base/basictypes.h"

namespace database{


  class DatabaseSession : public boost::noncopyable{
  public:
    DatabaseSession(boost::asio::io_service& io_service);
    virtual ~DatabaseSession();
    bool ConnectDatabase(const sql::SQLString& hostName, 
      const sql::SQLString& userName, const sql::SQLString& password);
    bool InsertProxyServer(const std::string domain, uint32 ip_addr, uint16 port);
    bool UpdateProxyServerPing(uint32 ip_addr);
    bool GetAllProxyServer();
  private:
    boost::asio::io_service& io_service_;
    sql::mysql::MySQL_Driver *driver_;
    boost::scoped_ptr<sql::Connection> sql_conn_;
  };

}

#endif // DATABASE_SERVER_DATABASE_SESSION_H_