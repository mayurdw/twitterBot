/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#include <stdlib.h>
#include "Utils.h"
#include "config.h"

static const UTIL_STR_ARRAY s_sConfigKeys = { "currentFilename", "daysToFileUpdate" };
static UTIL_STR_ARRAY s_sConfig = { 0, };
static xmlWrapperPtr psConfigFilePtr = _null_;

static void DebugConfig( void );
static ERROR_CODE InitConfig( void );

ERROR_CODE ReadConfig(void)
{
   ERROR_CODE eRet = NO_ERROR;
   uint32_t x = 0;

   eRet = OpenXmlFile( &psConfigFilePtr, CONFIG_FILENAME );
   if( eRet != NO_ERROR )
   {
      printf( "OpenXml returned = %d, Initialising config", eRet );
      InitConfig();
   }

   while( !ISERROR( eRet ) && x < CONFIG_LAST )
   {
      eRet = ExtractDataFromElement( psConfigFilePtr, s_sConfigKeys.aszStringArray[x], s_sConfig.aszStringArray[x], sizeof( s_sConfig.aszStringArray[x] ) );
      x++;
   }

   DebugConfig();

   CleanupDumpXmlMemory( );
   psConfigFilePtr = _null_;
   return eRet;
}

bool IsNewFileRequired()
{
   return ( atoi( s_sConfig.aszStringArray[CONFIG_DAYS_UNTIL_UPDATE] ) == 0 );
}

ERROR_CODE UpdateConfig( CONFIG_KEYS eConfigKey, const char * pszConfigValue )
{
    if( eConfigKey >= CONFIG_LAST || pszConfigValue == NULL || strlen( pszConfigValue ) == 0 )
    {
        return INVALID_ARG;
    }

    Strcpy_safe( s_sConfig.aszStringArray[eConfigKey], pszConfigValue, sizeof( s_sConfig.aszStringArray[eConfigKey] ) );

    return WriteXml( CONFIG_FILENAME, &s_sConfigKeys, &s_sConfig );
}

ERROR_CODE GetConfig( CONFIG_KEYS eConfigKey, char * pszConfigValue, uint32_t ulBufferSize )
{
    if( eConfigKey >= CONFIG_LAST || pszConfigValue == NULL || ulBufferSize == 0 )
    {
        return INVALID_ARG;
    }

    Strcpy_safe( pszConfigValue, s_sConfig.aszStringArray[eConfigKey], ulBufferSize );

    return NO_ERROR;
}

static void DebugConfig(void)
{
    printf("%s: Debugging config\n", __func__);
    for (CONFIG_KEYS x = CONFIG_CURRENT_FILENAME; x < CONFIG_LAST; x++)
    {
        printf("s_sConfig.aszStringArray[%d] = %s\n", x, s_sConfig.aszStringArray[x]);
    }
}

static ERROR_CODE InitConfig(void)
{
    GenerateFileName( s_sConfig.aszStringArray[0], sizeof( s_sConfig.aszStringArray[0] ) );
    Strcpy_safe( s_sConfig.aszStringArray[1], "0", sizeof( s_sConfig.aszStringArray[1] ) );
    
    return WriteXml( CONFIG_FILENAME, &s_sConfigKeys, &s_sConfig );
}
