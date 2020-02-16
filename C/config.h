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

ERROR_CODE ReadConfig( void );
bool IsNewFileRequired( void );

#endif