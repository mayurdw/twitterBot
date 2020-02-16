/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "Utils.h"
#include "xmlWrapper.h"

#define CONFIG_FILENAME         "config.xml"
#define DAYS_UNTIL_NEXT_UPDATE  "14"

ERROR_CODE ReadConfig( void );
bool IsNewFileRequired( void );
ERROR_CODE UpdateConfig( CONFIG_KEYS eConfigKey, const char * pszConfigValue );
#endif