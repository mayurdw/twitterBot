cmake_minimum_required(VERSION 3.10)

# set the project name
project(TwitterBot VERSION 1.0)

add_subdirectory(Utils)
include_directories(Utils)
include_directories(curl_windows/include)

# add the executable
add_executable(TwitterBot main.c config.c config.h)
target_link_libraries(TwitterBot curl_windows/lib/libcurl.a Utils )