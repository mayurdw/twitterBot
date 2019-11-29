/*
* Author: Mayur Wadhwani
* Date: 30th Novemeber 2019
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    NO_ERROR = 0,
} ERROR_CODE;

void LogDebug( const char *pszMessage, ... )
{
    char szLog[256 + 1] = { 0, };
    va_list ap;

    va_start(ap, pszMessage);
    vsnprintf( szLog, sizeof( szLog ), pszMessage, ap );
    va_end(ap);

    printf( szLog );
}

ERROR_CODE DownloadFeedFile( const char *pszURL )
{
    return NO_ERROR;
}

bool FeedFileExists( const char *pszFileName )
{
    return false;
}

ERROR_CODE GenerateFileName( char *pszFileName, uint32_t ulBufferSize )
{
    return NO_ERROR;
}

int main( )
{
    return 0;
}