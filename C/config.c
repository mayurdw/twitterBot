/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#include <stdlib.h>
#include "Utils.h"
#include "config.h"

// Preprocessors defines
#define CONFIG_FILENAME         ( "config.xml" )
#define DBG_CONFIG              ( 1 )

// Statics
static BOT_CONFIG s_sBotConfig = { 0, };
static const char *s_apszConfigKeys[] = { "currentFilename", "daysToFileUpdate" };

static const XML_ITEM s_apsConfigKeys[] = 
{
   XML_STR( "currentFilename",  BOT_CONFIG, szRssFilename      ),
   XML_STR( "daysToFileUpdate", BOT_CONFIG, szDaysUntilUpdate  ),
};

static void DebugConfig( void );
static ERROR_CODE Config_Reset( void );
static ERROR_CODE WriteConfig( const BOT_CONFIG *psBotConfig );
static ERROR_CODE Config_Read( void );

ERROR_CODE Config_Init( void )
{
   ERROR_CODE eRet = NO_ERROR;

   eRet = Config_Read();
   if( ISERROR( eRet ) )
   {
      DBG_PRINTF( "OpenXml returned = %d, Initialising config\n", eRet );
      eRet = Config_Reset();
   }

   DebugConfig();

   return eRet;
}

static ERROR_CODE Config_Read(void)
{
   ERROR_CODE eRet = NO_ERROR;

   eRet = xmlWrapperParseFile( CONFIG_FILENAME, s_apsConfigKeys, ARRAY_COUNT( s_apsConfigKeys ), &s_sBotConfig );

   return eRet;
}

bool IsNewFileRequired()
{
   return ( atoi( s_sBotConfig.szDaysUntilUpdate ) == 0 );
}

ERROR_CODE Config_GetRssFilename( char *pszFilename, uint32_t ulBufferSize )
{
   RETURN_ON_NULL( pszFilename );
   UTIL_ASSERT( ulBufferSize != 0, INVALID_ARG );

   return Strcpy_safe( pszFilename, s_sBotConfig.szRssFilename, ulBufferSize );
}

ERROR_CODE Config_GetDaysUntilUpdate( char *pszDaysUntilUpdate, uint32_t ulBufferSize )
{
   RETURN_ON_NULL( pszDaysUntilUpdate );
   UTIL_ASSERT( ulBufferSize != 0, INVALID_ARG );

   return Strcpy_safe( pszDaysUntilUpdate, s_sBotConfig.szDaysUntilUpdate, ulBufferSize );
}

ERROR_CODE Config_SetRssFilename( const char *pszFilename )
{
   RETURN_ON_NULL( pszFilename );

   RETURN_ON_FAIL( Strcpy_safe( s_sBotConfig.szRssFilename, pszFilename, sizeof( s_sBotConfig.szRssFilename ) ) );

   return WriteConfig( &s_sBotConfig );
}

ERROR_CODE Config_SetDaysUntilUpdate( const char *pszDaysUntilUpdate )
{
   RETURN_ON_NULL( pszDaysUntilUpdate );

   RETURN_ON_FAIL( Strcpy_safe( s_sBotConfig.szDaysUntilUpdate, pszDaysUntilUpdate, sizeof( s_sBotConfig.szDaysUntilUpdate ) ) );

   return WriteConfig( &s_sBotConfig );
}

static void DebugConfig(void)
{
#if DBG_CONFIG
   DBG_PRINTF( "------------------------------" );
   DBG_PRINTF( "Debugging config.xml" );
   DBG_PRINTF( "Current filename = %s", s_sBotConfig.szRssFilename );
   DBG_PRINTF( "Days Until Next Update = %s", s_sBotConfig.szDaysUntilUpdate );
   DBG_PRINTF( "------------------------------" );
#endif
}

static ERROR_CODE Config_Reset(void)
{
   BOT_CONFIG sBotConfig = { 0, };
   
   GenerateFileName( sBotConfig.szRssFilename, sizeof( sBotConfig.szRssFilename ) );
   Strcpy_safe( sBotConfig.szDaysUntilUpdate, "0", sizeof( sBotConfig.szDaysUntilUpdate ) );

   RETURN_ON_FAIL( WriteConfig( &sBotConfig ) );

   memset( &s_sBotConfig, 0, sizeof( BOT_CONFIG ) );
   s_sBotConfig = sBotConfig;

   return NO_ERROR;
}

static ERROR_CODE WriteConfig( const BOT_CONFIG *psBotConfig )
{
   xmlWriterPtrs sWriter = { 0, };

   RETURN_ON_NULL( psBotConfig );

   RETURN_ON_FAIL( CreateDocPtr( &sWriter ) );
   RETURN_ON_FAIL( CreateXmlNode( &sWriter, s_apszConfigKeys[0], psBotConfig->szRssFilename ) );
   RETURN_ON_FAIL( CreateXmlNode( &sWriter, s_apszConfigKeys[1], psBotConfig->szDaysUntilUpdate ) );
   RETURN_ON_FAIL( WriteXmlFile( &sWriter, CONFIG_FILENAME ) );

   return NO_ERROR;
}
