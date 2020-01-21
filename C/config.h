#ifndef CONFIG_H
#define CONFIG_H

#include "xmlWrapper.h"

#define CONFIG_FILENAME         "config.xml"

typedef enum 
{
  CONFIG_CURRENT_FILENAME = 0,
  CONFIG_DAYS_UNTIL_UPDATE,

  CONFIG_LAST
}CONFIG_KEYS;


ERROR_CODE ReadConfig( void );

#endif