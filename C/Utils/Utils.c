/*
* Author: Mayur Wadhwani
* Created: Feb 2020
*/

#include <time.h>
#include "Utils.h"

static const char* s_pszFileFormat = "%04i%02i%02i.xml";

ERROR_CODE GenerateFileName(char* pszFileName, uint32_t ulBufferSize)
{
    time_t sTime = { 0, };
    struct tm* psTimeNow = NULL;
    if (!pszFileName || ulBufferSize < MAX_FILENAME_LEN)
        return INVALID_ARG;

    memset(pszFileName, 0, ulBufferSize);
    time(&sTime);
    psTimeNow = localtime(&sTime);

    snprintf(pszFileName, ulBufferSize, s_pszFileFormat, psTimeNow->tm_year + 1900, psTimeNow->tm_mon + 1, psTimeNow->tm_mday);

    return SUCCESS;
}