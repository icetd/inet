cmake_minimum_required(VERSION 3.10)
project(inet)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_BUILD_TYPE Debug)

option(BUILD_TESTS "Enable testing" ON)

set (SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src)
set (INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/inc)

file(GLOB_RECURSE SRC_FILES
	"${SOURCE_DIR}/util/*.c*"
	"${SOURCE_DIR}/net/*.c*"
	"${SOURCE_DIR}/log/*.c*"
	"${SOURCE_DIR}/http/*.c*"
	"${SOURCE_DIR}/websocket/*.c*"
)

file (GLOB_RECURSE INC_FILES
	"${INCLUDE_DIR}/util/*.h"
	"${INCLUDE_DIR}/net/*.h"
	"${INCLUDE_DIR}/log/*.h"
	"${INCLUDE_DIR}/http/*.h"
	"${INCLUDE_DIR}/websocket/*.h"
)

include_directories(
	"${INCLUDE_DIR}/util"
	"${INCLUDE_DIR}/net"
	"${INCLUDE_DIR}/log"
	"${INCLUDE_DIR}/http"
	"${INCLUDE_DIR}/websocket"
)

add_library(${PROJECT_NAME}_static STATIC ${SRC_FILES})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "inet")
set_target_properties(${PROJECT_NAME}_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "inet")
set_target_properties(${PROJECT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

if(BUILD_TESTS)
add_executable(testTcpServer test/testTcpServer.cc)
target_link_libraries(testTcpServer ${PROJECT_NAME} pthread)

add_executable(testTcpClient test/testTcpClient.cc)
target_link_libraries(testTcpClient ${PROJECT_NAME} pthread)

add_executable(testHttpServer test/testHttpServer.cc)
target_link_libraries(testHttpServer ${PROJECT_NAME} pthread)

add_executable(testWebSocketServer test/testWebSocketServer.cc)
target_link_libraries(testWebSocketServer ${PROJECT_NAME} pthread)
endif()


file(GLOB_RECURSE APP_SRC_FILES
	"${CMAKE_CURRENT_LIST_DIR}/staticwebserver/*.c*"
)
include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/staticwebserver"
)
add_executable(StaticWebServer ${APP_SRC_FILES})
target_link_libraries(StaticWebServer ${PROJECT_NAME} pthread)
file(COPY www DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/configs DESTINATION ${CMAKE_BINARY_DIR})


# out set
set (OUT_BIN ${CMAKE_BINARY_DIR}/target/lib)
set (OUT_LIB ${CMAKE_BINARY_DIR}/target/lib)
set (OUT_INC ${CMAKE_BINARY_DIR}/target/include)


set_target_properties(${PROJECT_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${OUT_LIB}"
  LIBRARY_OUTPUT_DIRECTORY "${OUT_LIB}"
  RUNTIME_OUTPUT_DIRECTORY "${OUT_BIN}"
)

add_custom_target(copy_files ALL  
	VERBATIM
	COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_INC}"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/inc" "${OUT_INC}"
)
