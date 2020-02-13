/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#include <dirent.h>
#include <time.h>
#include "Utils.h"

ERROR_CODE GenerateFileName(char* pszFileName, uint32_t ulBufferSize)
{
    const char* pszFileFormat = "%04i%02i%02i.xml";
    time_t sTime = { 0, };
    struct tm* psTimeNow = NULL;
    if (!pszFileName || ulBufferSize < MAX_FILENAME_LEN)
        return INVALID_ARG;

    memset(pszFileName, 0, ulBufferSize);
    time(&sTime);
    psTimeNow = localtime(&sTime);

    snprintf(pszFileName, ulBufferSize, pszFileFormat, psTimeNow->tm_year + 1900, psTimeNow->tm_mon + 1, psTimeNow->tm_mday);

    return NO_ERROR;
}