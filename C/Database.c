/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

// Macros
#define MAX_BLOG_POSTS  200

// typedefs 
typedef struct DATABASE
{
   BLOG_POST asList[MAX_BLOG_POSTS];
}DATABASE;

// Static variables
static DATABASE s_sList = { 0, };

// Static functions
static ERROR_CODE CreateDatabaseFile( void );
static ERROR_CODE ReadDatabaseFile( void );
static ERROR_CODE ReadFeedXmlFile( const char *pszFileName );

ERROR_CODE Database_Init( void )
{
   ERROR_CODE eRet = NO_ERROR;

   eRet = ReadDatabaseFile();
   if( ISERROR( eRet ) )
   {
      char szRSSfeedFile[MAX_FILENAME_LEN + 1] = { 0, };

      // Try to instantiate the database file from xml file
      RETURN_ON_FAIL( Config_GetRssFilename( szRSSfeedFile, sizeof( szRSSfeedFile ) ) );

      eRet = ReadFeedXmlFile( szRSSfeedFile );
      // Create Database file
   }

   return eRet;
}


static ERROR_CODE ReadFeedXmlFile( const char *pszFileName )
{
   const XML_ITEM asPost[] = 
   {
      XML_STR( "title", BLOG_POST, szTitle ),
      XML_STR( "link", BLOG_POST, szLink )
   };

   const XML_ITEM asPosts[] =
   {
      XML_ARRAY( "item", DATABASE, asList, asPost, ARRAY_COUNT( asPost ), ARRAY_COUNT( s_sList.asList ) )
   };

   memset( &s_sList, 0, sizeof( s_sList ) );

   RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, asPosts, ARRAY_COUNT( asPosts ), &s_sList ) );

   return NO_ERROR;
}

ERROR_CODE CreateDatabaseFile( void )
{
   return FAILED;
}

ERROR_CODE ReadDatabaseFile( void )
{
   return FAILED;
}