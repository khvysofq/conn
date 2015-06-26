# -G "Visual Studio 9 2008"
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(proxy_system)
set_property(GLOBAL PROPERTY USE_FOLDERS On)

##########################################################
## Step 1 
SET(VZ_S_BASE_DIR $ENV{VZ_SBASE_PATH})
SET(BOOST_1_56_DIR "/root/boost/boost_1_56_0")
# With VS2013 only support by boost 1.56
if(WIN32 AND MSVC12)
	SET(BOOST_1_56_DIR $ENV{BOOST_1_56_0_PATH})
endif()

##1. INCLUDE_DIRECTORES
if(UNIX)
	SET(LIBVZNET_INCLUDE_DIR
	${PROJECT_SOURCE_DIR}/src
	${BOOST_1_56_DIR}
	)
elseif(WIN32)
	SET(LIBVZNET_INCLUDE_DIR
	${BOOST_1_56_DIR}
	${PROJECT_SOURCE_DIR}/src
	${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/src/windows
	)
endif()

MESSAGE(STATUS "       Add library path and name")
IF(UNIX)
	MESSAGE(STATUS "       LINUX")
	SET(LIBVZNET_LIBRARY_DIR
		${PROJECT_SOURCE_DIR}/lib/unix
		${BOOST_1_56_DIR}/stage/lib
		)
	SET(LIBVZNET_LINK_LIB
		libboost_thread.a
		libboost_system.a
		libvzconn.a
		pthread
		rt
		#libglog.a
		#libgflags.a
		#libunwind.a 
		#liblzma.a
		)
ELSEIF(WIN32)
	if(CMAKE_BUILD_TYPE MATCHES DEBUG)
		MESSAGE(STATUS "       Debug Mode")
		MESSAGE(STATUS "       MSVC12")
		SET(LIBVZNET_LIBRARY_DIR
			${BOOST_1_56_DIR}/stage/lib
			${PROJECT_SOURCE_DIR}/lib/win32
			${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/vs2013/
			)
		SET(LIBVZNET_LINK_LIB
			ws2_32.lib
			libvzconn.lib
			)
	elseif(CMAKE_BUILD_TYPE MATCHES RELEASE)
		MESSAGE(STATUS "       Release Mode")
		MESSAGE(STATUS "       MSVC12")
		SET(LIBVZNET_LIBRARY_DIR
			${BOOST_1_56_DIR}/stage/lib
			${PROJECT_SOURCE_DIR}/lib
			${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/vs2013/
			)
		SET(LIBVZNET_LINK_LIB
			ws2_32.lib
			libvzconn.lib
			)
	endif()
ENDIF()

MESSAGE(STATUS "Step 4 : Add code source")
if(UNIX)
	ADD_DEFINITIONS(-DGOOGLE_GLOG_DLL_DECL=)
elseif(WIN32)
	ADD_DEFINITIONS(
	-DGOOGLE_GLOG_DLL_DECL=
	-DGLOG_NO_ABBREVIATED_SEVERITIES
	-D_CRT_SECURE_NO_WARNINGS
	-D_SCL_SECURE_NO_WARNINGS
	-D_WINSOCK_DEPRECATED_NO_WARNINGS
	-D_WIN32_WINNT=0x0502
	)
endif()

INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
#####################################################################
# Step 3 :Set visual studio runtime type
if(WIN32)
	set(CompilerFlags
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			)
	foreach(CompilerFlag ${CompilerFlags})
	  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	  string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
	  #string(REPLACE "/EDITANDCONTINUE" "/SAFESEH" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
	
################################################################
	
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/win32")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/win32")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/win32")
	
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/win32")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/win32")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/win32")
		 
	# With Release properties
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/win32")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/win32")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/win32")
elseif(UNIX)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/unix")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/unix")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/unix")

	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/unix")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/unix")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/unix")
	 
	# With Release properties
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/unix")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/unix")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/unix")
ENDIF()
##########################################################
## Setp 3
ADD_SUBDIRECTORY(src/normal_proxy)
ADD_SUBDIRECTORY(src/proxy_client)
ADD_SUBDIRECTORY(src/fake_proxy_client)
ADD_SUBDIRECTORY(src/fake_proxy_server)
ADD_SUBDIRECTORY(src/database_server)
ADD_SUBDIRECTORY(src/vzconn)