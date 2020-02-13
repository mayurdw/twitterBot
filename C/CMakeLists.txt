cmake_minimum_required(VERSION 3.10)

# set the project name
project(TwitterBot VERSION 1.0)

add_subdirectory(Utils)
include_directories(Utils)

# add the executable
set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)
add_executable(TwitterBot main.c config.c config.h)
include_directories(${CURL_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
target_link_libraries(TwitterBot ${CURL_LIBRARIES} Utils ${LIBXML2_LIBRARIES} )