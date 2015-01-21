cmake_minimum_required(VERSION 2.8)

project(ptree-utils)

# NOTE: Uses environment variable $BOOST_ROOT to find the boost package.
find_package(Boost 
  1.51.0
  REQUIRED)

if (Boost_FOUND)
  message("Boost include path '${Boost_INCLUDE_DIRS}'\n")
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(ptree-test main.cpp)  
endif()
