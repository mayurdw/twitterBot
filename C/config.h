/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "Utils.h"
#include "xmlWrapper.h"

/* 
    Stores the config for the bot
    Each config item should only be a string
 */
typedef struct
{
    // RSS file which is downloaded from the blogsite
    char szRssFilename[MAX_FILENAME_LEN + 1];
    // Decrementing counter until Bot downloads a new RSS file
    char szDaysUntilUpdate[2 + 1];
} BOT_CONFIG;

/* 
    Initializes the Bot config on startup
    Will try to read config file
    If that fails, will create a default config file
    Will also log the current config if debugging is enabled
    @param:         NONE
    @return:        NO_ERROR    -> Success
 */
ERROR_CODE Config_Init(void);

/* 
    Checks config parameter szDaysUntilUpdate
    @param:         NONE
    @return:        true    -> szDaysUntilUpdate is 0 or empty
    @return:        false   -> szDaysUntilupdate is not 0 or empty
 */
bool IsNewFileRequired(void);

/* 
    Gets filename of the downloaded RSS file
    @param(OUTPUT):     pszFilename     -> Filename of the RSS file
    @param(INPUT):      ulBufferSize    -> Buffer size of pszFilename
    @return:            NO_ERROR        -> Success
    @return:            INVALID_ARG     -> One or both parameters are invalid
 */
ERROR_CODE Config_GetRssFilename(char *pszFilename, uint32_t ulBufferSize);

/* 
    Gets days until update for the downloaded RSS file
    @param(INPUT):      pszDaysUntilUpdate      -> String to hold the number of days
    @param(INPUT):      ulBufferSize            -> Buffer size
    @return:            NO_ERROR                -> Success
    @return:            INVALID_ARG             -> One or both parameters are invalid
 */
ERROR_CODE Config_GetDaysUntilUpdate(char *pszDaysUntilUpdate, uint32_t ulBufferSize);

/* 
    Sets filename of the downloaded RSS file
    @param(INPUT):      pszFilename     -> Filename of the RSS file
    @return:            NO_ERROR        -> Success
    @return:            INVALID_ARG     -> Input parameters are invalid
 */
ERROR_CODE Config_SetRssFilename(const char *pszFilename);

/* 
    Sets daysUntilUpdate for the downloaded RSS file
    @param(INPUT):      pszDaysUntilUpdate  -> Filename of the RSS file
    @return:            NO_ERROR            -> Success
    @return:            INVALID_ARG         -> Input parameters are invalid
 */
ERROR_CODE Config_SetDaysUntilUpdate(const char *pszDaysUntilUpdate);

#endif