#include "database_session.h"
#include "database_server_defines.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

namespace database{

  DatabaseSession::DatabaseSession(boost::asio::io_service& io_service)
  :io_service_(io_service),
  driver_(NULL){
  }

  DatabaseSession::~DatabaseSession(){

  }

  bool DatabaseSession::ConnectDatabase(const sql::SQLString& hostName,
    const sql::SQLString& userName, const sql::SQLString& password){
    try{
      driver_ = sql::mysql::get_mysql_driver_instance();
      BOOST_ASSERT(driver_ != NULL);
      sql_conn_.reset(driver_->connect(hostName, userName, password));
    }
    catch (std::exception& e){
      LOG_ERROR << "Connect database error " << e.what() << std::endl;
      return false;
    }
    if (sql_conn_->isValid()){
      return true;
    }
    return false;
  }

  bool DatabaseSession::InsertProxyServer(
    const std::string domain, uint32 ip_addr, uint16 port){
    if (!sql_conn_ || !sql_conn_->isValid()){
      sql_conn_.reset();
      return false;
    }
    try{
      char temp_sql[512];
      sprintf(temp_sql, INSERT_PROXY_SERVER, domain.c_str(), ip_addr, port, 
        PROXY_STATE_ONLINE, PRoXY_COMMENT_USA);
      boost::scoped_ptr<sql::PreparedStatement> sql_prepared_statement(
        sql_conn_->prepareStatement(temp_sql));
      sql_prepared_statement->executeQuery();
    }
    catch (std::exception& e){
      LOG_ERROR << "Insert proxyserver error " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  bool DatabaseSession::UpdateProxyServerPing(uint32 ip_addr){
    if (!sql_conn_ || !sql_conn_->isValid()){
      sql_conn_.reset();
      return false;
    }
    try{
      char temp_sql[512];
      sprintf(temp_sql, UPDATE_PROXY_SERVER_PING, ip_addr);
      boost::scoped_ptr<sql::PreparedStatement> sql_prepared_statement(
        sql_conn_->prepareStatement(temp_sql));
      sql_prepared_statement->executeQuery();
    }
    catch (std::exception& e){
      LOG_ERROR << "Insert proxyserver error " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  bool DatabaseSession::GetAllProxyServer(){
    if (!sql_conn_ || !sql_conn_->isValid()){
      sql_conn_.reset();
      return false;
    }
    try{
      boost::scoped_ptr<sql::PreparedStatement> sql_prepared_statement(
        sql_conn_->prepareStatement("SELECT * FROM proxy_db.proxy_server_infor"));
      boost::scoped_ptr<sql::ResultSet> sql_result(sql_prepared_statement->executeQuery());
      sql_result->afterLast();
      while (sql_result->previous()){
        LOG_INFO << sql_result->getUInt(PROXY_ID) << std::endl;
        LOG_INFO << sql_result->getString(PROXY_DOMAIN).c_str() << std::endl;
        LOG_INFO << sql_result->getUInt(PROXY_IPADDRESS) << std::endl;
        LOG_INFO << sql_result->getUInt(PROXY_PORT) << std::endl;
        //uint64 date = sql_result->getUInt64(PROXY_LAST_UPDATE);
        //LOG_INFO << date << std::endl;
        LOG_INFO << sql_result->getUInt(PROXY_STATE) << std::endl;
        LOG_INFO << sql_result->getUInt(PROXY_COMMENT) << std::endl;
      }
    }
    catch (std::exception& e){
      LOG_ERROR << "Insert proxyserver error " << e.what() << std::endl;
      return false;
    }
    return true;
  }
}