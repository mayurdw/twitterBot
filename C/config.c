/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#include <stdlib.h>
#include "Utils.h"
#include "config.h"

static const UTIL_STR_ARRAY s_sConfigKeys = { "currentFilename", "daysToFileUpdate" };
static UTIL_STR_ARRAY s_sConfig = { 0, };

static void DebugConfig( void );
static ERROR_CODE InitConfig( void );

ERROR_CODE ReadConfig(void)
{
    ERROR_CODE eRet = ReadXml( CONFIG_FILENAME, &s_sConfigKeys, &s_sConfig );

    if( eRet != NO_ERROR )
    {
        printf( "ReadXml returned = %d, Initialising config", eRet );
        InitConfig();
    }
    DebugConfig();
    return eRet;
}

bool IsNewFileRequired()
{
    return ( ( strlen( s_sConfig.aszStringArray[CONFIG_DAYS_UNTIL_UPDATE] ) > 0 ) && ( atoi( s_sConfig.aszStringArray[CONFIG_DAYS_UNTIL_UPDATE] ) <= 0 ) );
}

ERROR_CODE UpdateConfig( CONFIG_KEYS eConfigKey, const char * pszConfigValue )
{
    if( eConfigKey >= CONFIG_LAST || pszConfigValue == NULL || strlen( pszConfigValue ) == 0 )
    {
        return INVALID_ARG;
    }

    memset( s_sConfig.aszStringArray[eConfigKey], 0, sizeof( s_sConfig.aszStringArray[eConfigKey] ) );
    strcpy( s_sConfig.aszStringArray[eConfigKey], pszConfigValue );

    return WriteXml( CONFIG_FILENAME, &s_sConfigKeys, &s_sConfig );
}

static void DebugConfig(void)
{
    printf("%s: Debugging config\n", __func__);
    for (CONFIG_KEYS x = 0; x < CONFIG_LAST; x++)
    {
        printf("s_sConfig.aszStringArray[%d] = %s\n", x, s_sConfig.aszStringArray[x]);
    }
}

static ERROR_CODE InitConfig(void)
{
    GenerateFileName( s_sConfig.aszStringArray[0], sizeof( s_sConfig.aszStringArray[0] ) );
    strcpy( s_sConfig.aszStringArray[1], "0" );

    return WriteXml( CONFIG_FILENAME, &s_sConfigKeys, &s_sConfig );
}
