#ifndef DATABASE_SERVER_DEFINES_H_
#define DATABASE_SERVER_DEFINES_H_

#include <iostream>

#define LOG_INFO std::cerr << "I[" << __FUNCTION__ << " Line " << __LINE__ << "] "
#define LOG_WARNING std::cerr << "W[" << __FUNCTION__ << " Line " << __LINE__ << "] "
#define LOG_ERROR std::cerr << "E[" << __FUNCTION__ << " Line " << __LINE__ << "] "
namespace database{
  // string
  const char PROXY_DOMAIN[] = "proxy_domain";
  // Int
  const char PROXY_IPADDRESS[] = "proxy_ipaddress";
  // Int
  const char PROXY_PORT[] = "proxy_port";
  // String
  const char PROXY_LAST_UPDATE[] = "last_update";
  // Int
  const char PROXY_STATE[] = "state";
  // Int
  const char PROXY_COMMENT[] = "comment";
  // Uint
  const char PROXY_ID[] = "id";


  const char INSERT_PROXY_SERVER[] = "INSERT INTO `proxy_db`.`proxy_server_infor` ("
    "`proxy_domain`, "
    "`proxy_ipaddress`, "
    "`proxy_port`,"
    "`last_update`,"
    "`state`, "
    "`comment`) VALUES ("
    "'%s',%d,%d,NOW(),%d,%d);";

  const char UPDATE_PROXY_SERVER_PING[] =
    "UPDATE `proxy_db`.`proxy_server_infor` SET `last_update`=NOW() WHERE `proxy_ipaddress`='%d'";

  const int PROXY_STATE_ONLINE = 0;
  const int PROXY_STATE_OFFLINE = 1;

  const int PRoXY_COMMENT_USA = 0;
}
#endif // DATABASE_SERVER_DEFINES_H_