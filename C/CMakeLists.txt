cmake_minimum_required(VERSION 3.10)

# set the project name
project(TwitterBot VERSION 1.0)

# add the executable
set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
add_executable(TwitterBot main.c)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(TwitterBot ${CURL_LIBRARIES})