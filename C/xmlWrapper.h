
#ifndef XMLWRAPPER_H
#define XMLWRAPPER_H


#include <stdint.h>

typedef enum
{
    NO_ERROR = 0,
    INVALID_ARG,
    FILE_ERROR,
    CONFIG_VALIDATION_ERROR,
} ERROR_CODE;

ERROR_CODE ReadXml( const char *pszFilename, const char **ppaszList, uint32_t ulArraySize );

#endif