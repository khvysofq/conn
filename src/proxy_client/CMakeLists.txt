cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "proxy")

#########################################################################
#INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
#LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/conn/proxy_server.h
	${CMAKE_CURRENT_SOURCE_DIR}/conn/proxy_server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconn.h
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconnpair.h
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconnpair.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/proxybase.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/http/http_parser.h
	${CMAKE_CURRENT_SOURCE_DIR}/http/http_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http/reply.h
	${CMAKE_CURRENT_SOURCE_DIR}/http/reply.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http/header.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakedefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconn.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserverconn.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserverconn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeparser.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeparser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconnsession.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconnsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientserver.cpp
	
	)
SOURCE_GROUP(http FILES
	${CMAKE_CURRENT_SOURCE_DIR}/http/http_parser.h
	${CMAKE_CURRENT_SOURCE_DIR}/http/http_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http/reply.h
	${CMAKE_CURRENT_SOURCE_DIR}/http/reply.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/http/header.h
)
SOURCE_GROUP(conn FILES
	${CMAKE_CURRENT_SOURCE_DIR}/conn/proxy_server.h
	${CMAKE_CURRENT_SOURCE_DIR}/conn/proxy_server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconn.h
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconnpair.h
	${CMAKE_CURRENT_SOURCE_DIR}/conn/httpconnpair.cpp
)
SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/proxybase.h
)
SOURCE_GROUP(fake FILES
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakedefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconn.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserverconn.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeserverconn.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeparser.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeparser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconnsession.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientconnsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/fake/fakeclientserver.cpp
)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
)