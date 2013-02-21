cmake_minimum_required(VERSION 2.8)

project(scracc)

set(CMAKE_CXX_FLAGS "-std=c++11")

find_package(Boost COMPONENTS filesystem system REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found! Consider installing libboost-filesystem-dev.")
endif ()

add_subdirectory(libscracc)
add_subdirectory(scracc)

set (CPACK_GENERATOR "DEB")
set (CPACK_PACKAGE_NAME "scracc")
set (CPACK_PACKAGE_VERSION "0.1.1")

set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Gabor Fekete")
set (CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-filesystem1.49.0 (>= 1.49.0-3.1ubuntu1), libcrypto++9 (>= 5.6.1-6)")
set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "The Scracc C++ Prototyping Tool.")
set (CPACK_DEBIAN_PACKAGE_SECTION "devel")

include(CPack)
