include_directories(${CURL_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
add_library(Utils xmlWrapper.c xmlWrapper.h Utils.c Utils.h CurlWrapper.c CurlWrapper.h)