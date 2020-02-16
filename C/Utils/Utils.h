/*
* Author: Mayur Wadhwani
* Created: January 2020
*/

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_FILENAME_LEN        16

typedef enum
{
    CONFIG_CURRENT_FILENAME = 0,
    CONFIG_DAYS_UNTIL_UPDATE,

    CONFIG_LAST
}CONFIG_KEYS;

typedef enum
{
    NO_ERROR = 0,
    INVALID_ARG,
    FILE_ERROR,
    CONFIG_VALIDATION_ERROR,
} ERROR_CODE;

typedef struct
{
    /* data */
    char aszStringArray[CONFIG_LAST][MAX_FILENAME_LEN + 1];
} UTIL_STR_ARRAY;

#define RETURN(x)           (return x);
#define ISERROR(x)          (x != NO_ERROR)
#define RETURN_ON_FAIL(x)   { if( ISERROR( x ) ) { printf( "Error eRet = %d", x ); return x; }}

ERROR_CODE GenerateFileName(char* pszFileName, uint32_t ulBufferSize);

#endif