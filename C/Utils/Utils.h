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
    NO_ERROR = 0,
    INVALID_ARG,
    FILE_ERROR,
    CONFIG_VALIDATION_ERROR,
    NOT_FOUND,
} ERROR_CODE;

#define _null_              0
#define RETURN(x)           (return x);
#define ISERROR(x)          (x != NO_ERROR)
#define RETURN_ON_FAIL(x)   { if( ISERROR( x ) ) { printf( "%d|%s: Error eRet = %d\n", __LINE__, __func__, x ); return x; }}
#define RETURN_ON_NULL(x)   { if( x == _null_ ) RETURN_ON_FAIL( INVALID_ARG ); }
#define UTIL_ASSERT(x,y)    { if( !x ) return y; }

ERROR_CODE Strcpy_safe( char* pszDest, const char* pszSrc, uint32_t ulBufferSize );
ERROR_CODE GenerateFileName(char* pszFileName, uint32_t ulBufferSize);

#endif