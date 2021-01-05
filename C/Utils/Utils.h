/*
* Author: Mayur Wadhwani
* Created: January 2020
*/

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_FILENAME_LEN 16

/* 
    ERROR CODES to be used internally
 */
typedef enum
{
    NO_ERROR = 0,               // Success
    INVALID_ARG,                // Arg passed into the function are invalid
    FILE_ERROR,                 // File operation error
    CONFIG_VALIDATION_ERROR,    // Config invalid
    NOT_FOUND,                  // Missing
    TEST_FAILED,                // Test failed
    OVERFLOW,                   // Array or structure full, need to expand
} ERROR_CODE;

#define _null_ 0
#if _DEBUG
#define DBG_PRINTF(x, ...)                                \
    {                                                     \
        Dbg_printf(__func__, __LINE__, x, ##__VA_ARGS__); \
    }
#define DBG_INIT()  \
    {               \
        Dbg_Init(); \
    }
#else
#define DBG_PRINTF(x, ...)                                \
    {                                                     \
    }
#define DBG_INIT()  \
    {               \
    }

#endif
#define UTIL_ASSERT(x, y) \
    {                     \
        if (!x)           \
            return y;     \
    }
#define RETURN_ON_NULL(x)                \
    {                                    \
        if (x == _null_)                 \
            RETURN_ON_FAIL(INVALID_ARG); \
    }
#define ISERROR(x) (x != NO_ERROR)
#define RETURN(x) (return x);
#define RETURN_ON_FAIL(x)                     \
    {                                         \
        if (ISERROR(x))                       \
        {                                     \
            DBG_PRINTF("Error eRet = %d", x); \
            return x;                         \
        }                                     \
    }

#define ARRAY_COUNT(x) sizeof(x) / sizeof(x[0])

/* 
    Safe Strcpy function to prevent buffer overflow
    @param[OUT] pszDest: Destination buffer pointer
    @param[IN]  pszSrc:  String source pointer
    @param[IN] ulBufferSize: Buffer size of the destination

    @return: NO_ERROR: Success
    @return: INVALID_ARG: If args are invalid
 */
ERROR_CODE Strcpy_safe(char *pszDest, const char *pszSrc, uint32_t ulBufferSize);
/* 
    Generates a RSS file name for a fresh CURL download
    @param[OUT] pszFilename: Filename to be used
    @param[IN] ulBufferSize: Size of the filename buffer

    @return: NO_ERROR: Success
    @return: INVALID_ARG: If args are invalid
 */
ERROR_CODE GenerateFileName(char *pszFileName, uint32_t ulBufferSize);

/* 
    NOT TO BE CALLED DIRECTLY. USE DBG_PRINTF() macro
    Prints internal Debug 
    @param[IN]: pszFunc: Function from which macro is called
    @param[IN]: iLine:   Line number of the macro call
    @param[IN]: pszFormat: Format of the debug line
 */
void Dbg_printf(const char *pszFunc, int iLine, char *pszFormat, ...);
/* 
    NOT TO BE CALLED DIRECTLY, USE DBG_INIT() macro
    Initialises App Debug
 */
void Dbg_Init(void);


#endif