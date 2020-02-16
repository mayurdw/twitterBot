include_directories(${LIBXML2_INCLUDE_DIR})
add_library(Utils xmlWrapper.c xmlWrapper.h Utils.c Utils.h)