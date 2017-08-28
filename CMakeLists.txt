cmake_minimum_required(VERSION 2.8)

project(CPPXML)

# silence policy CMP0042 warning by enabling RPATH explicitly
if(APPLE AND NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH TRUE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CPPXML_3RDPARTY_DIR ${CPPXML_SOURCE_DIR}/3rdparty)
set(BUILD_SHARED_LIBS ON)

include_directories(
  ${CPPXML_3RDPARTY_DIR}/rapidxml-1.13
  ${CMAKE_INSTALL_PREFIX}/openresty/luajit/include/luajit-2.1
)
link_directories(${CMAKE_INSTALL_PREFIX}/openresty/luajit/lib)

add_subdirectory(src)
