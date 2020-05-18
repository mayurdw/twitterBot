/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "Utils.h"
#include "xmlWrapper.h"

typedef struct
{
   char szRssFilename[MAX_FILENAME_LEN + 1];
   char szDaysUntilUpdate[2 + 1];
} BOT_CONFIG;

ERROR_CODE ReadConfig( void );
bool IsNewFileRequired( void );

// Getters & setters
ERROR_CODE Config_GetRssFilename( char *pszFilename, uint32_t ulBufferSize );
ERROR_CODE Config_GetDaysUntilUpdate( char *pszDaysUntilUpdate, uint32_t ulBufferSize );
ERROR_CODE Config_SetRssFilename( const char *pszFilename );
ERROR_CODE Config_SetDaysUntilUpdate( const char *pszDaysUntilUpdate );

#endif