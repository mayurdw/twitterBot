/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include "Utils.h"

ERROR_CODE Strcpy_safe( char* pszDest, const char* pszSrc, uint32_t ulBufferSize )
{
   uint32_t ulCopySize = 0;
   RETURN_ON_NULL( pszDest );
   RETURN_ON_NULL( pszSrc );
   UTIL_ASSERT( ulBufferSize > 0, INVALID_ARG );

   memset( pszDest, 0, ulBufferSize );
   if( ulBufferSize < ( strlen( pszSrc ) + 1 ) )
   {
      ulCopySize = ulBufferSize - 1;
      printf( "Destination size is %d, source is %lu. Truncating\n", ulBufferSize, strlen( pszSrc ) );
   }
   else
   {
      ulCopySize = strlen( pszSrc );
   }

   strncpy( pszDest, pszSrc, ulCopySize );

   return NO_ERROR;
}

ERROR_CODE GenerateFileName(char* pszFileName, uint32_t ulBufferSize)
{
    const char* pszFileFormat = "%04i%02i%02i.xml";
    time_t sTime = { 0, };
    struct tm* psTimeNow = _null_;

    RETURN_ON_NULL( pszFileName );
    UTIL_ASSERT( ulBufferSize >= MAX_FILENAME_LEN, INVALID_ARG );

    memset(pszFileName, 0, ulBufferSize);
    time(&sTime);
    psTimeNow = localtime(&sTime);

    snprintf(pszFileName, ulBufferSize, pszFileFormat, psTimeNow->tm_year + 1900, psTimeNow->tm_mon + 1, psTimeNow->tm_mday);

    return NO_ERROR;
}

