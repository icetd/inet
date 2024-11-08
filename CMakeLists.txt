cmake_minimum_required(VERSION 3.10)
project(inet)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_BUILD_TYPE Debug)

set (SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src)
set (INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/inc)

file(GLOB_RECURSE SRC_FILES
	"${SOURCE_DIR}/util/*.cc"
	"${SOURCE_DIR}/net/*.cc"
	"${SOURCE_DIR}/log/*.cc"
)

file (GLOB_RECURSE INC_FILES
	"${INCLUDE_DIR}/util/*.h"
	"${INCLUDE_DIR}/net/*.h"
	"${INCLUDE_DIR}/log/*.h"
)

include_directories(
	"${INCLUDE_DIR}/util"
	"${INCLUDE_DIR}/net"
	"${INCLUDE_DIR}/log"
)

message("${INC_FILES}")

add_library(${PROJECT_NAME}_static STATIC ${SRC_FILES})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "inet")
set_target_properties(${PROJECT_NAME}_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "inet")
set_target_properties(${PROJECT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
add_executable(${PROJECT_NAME}_test test/test.cc)

target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} pthread)

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
	COMMAND ${CMAKE_COMMAND} -E
                  copy_directory
                  "${CMAKE_CURRENT_LIST_DIR}/inc"
                  "${OUT_INC}"
)
