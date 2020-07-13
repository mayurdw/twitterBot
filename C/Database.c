/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

// Macros
#define MAX_BLOG_POSTS  ( 200 )
#define DATABASE_FILE   ( "database.xml" )
#define DEBUG_DATABASE  ( 0 )

// typedefs 
typedef struct DATABASE
{
   char szPostCount[3+1];
   BLOG_POST asList[MAX_BLOG_POSTS];
}DATABASE;

// Static variables
static DATABASE s_sList = { 0, };

static const XML_ITEM s_asPost[] = 
{
   XML_STR( "title", BLOG_POST, szTitle ),
   XML_STR( "link", BLOG_POST, szLink ),
   XML_STR( "times_shared", BLOG_POST, szTimesShared )
};

static const XML_ITEM s_asPosts[] =
{
   XML_STR( "count", DATABASE, szPostCount ),
   XML_ARRAY( "post", DATABASE, asList, s_asPost, ARRAY_COUNT( s_asPost ), ARRAY_COUNT( s_sList.asList ) )
};

// Static functions
static ERROR_CODE CreateDatabaseFile( void );
static ERROR_CODE ReadDatabaseFile( void );
static ERROR_CODE ReadFeedXmlFile( const char *pszFileName );
static ERROR_CODE DebugDatabaseFile( void );

ERROR_CODE Database_Init( void )
{
   ERROR_CODE eRet = NO_ERROR;

   eRet = ReadDatabaseFile();
   if( ISERROR( eRet ) )
   {
      char szRSSfeedFile[MAX_FILENAME_LEN + 1] = { 0, };

      // Try to instantiate the database file from xml file
      RETURN_ON_FAIL( Config_GetRssFilename( szRSSfeedFile, sizeof( szRSSfeedFile ) ) );

      RETURN_ON_FAIL( ReadFeedXmlFile( szRSSfeedFile ) );
      // Create Database file
      eRet = CreateDatabaseFile( );
   }

   return eRet;
}


static ERROR_CODE ReadFeedXmlFile( const char *pszFileName )
{
   const XML_ITEM asRssPosts[] =
   {
      XML_ARRAY( "item", DATABASE, asList, s_asPost, ARRAY_COUNT( s_asPost ), ARRAY_COUNT( s_sList.asList ) )
   };

   memset( &s_sList, 0, sizeof( s_sList ) );

   RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, asRssPosts, ARRAY_COUNT( asRssPosts ), &s_sList ) );

   return NO_ERROR;
}

ERROR_CODE CreateDatabaseFile( void )
{
   uint32_t x = 0;
   ERROR_CODE eRet = NO_ERROR;

   // Calculate the number of posts
   for( x = ARRAY_COUNT( s_sList.asList ); x > 0 && ( strlen( s_sList.asList[x - 1].szTitle ) == 0 ); x-- );
   if( x != 0 )
   {
      DBG_PRINTF( "Writing [%u] posts onto the database file", x );
      snprintf( s_sList.szPostCount, sizeof( s_sList.szPostCount ), "%u", x );
      eRet = xmlWrapperWriteFile( DATABASE_FILE, s_asPosts, ARRAY_COUNT( s_asPosts ), &s_sList );

      DebugDatabaseFile( );
   }
   else
   {
      eRet = FAILED;
   }

   return eRet;
}

ERROR_CODE ReadDatabaseFile( void )
{
   memset( &s_sList, 0, sizeof( s_sList ) );
   RETURN_ON_FAIL( xmlWrapperParseFile( DATABASE_FILE, s_asPosts, ARRAY_COUNT( s_asPosts ), &s_sList ) );

   return DebugDatabaseFile();
}

ERROR_CODE DebugDatabaseFile( void )
{
#if DEBUG_DATABASE
   uint32_t ulCount = atoi( s_sList.szPostCount );

   DBG_PRINTF( "Listing [%u] posts", ulCount );
   for( int x = 0; x < ulCount; x++ )
   {
      DBG_PRINTF( "----------------------------------------" );
      DBG_PRINTF( "Item#         = [%d]", x );
      DBG_PRINTF( "Title         = [%0.20s]", s_sList.asList[x].szTitle );
      DBG_PRINTF( "Link          = [%0.20s]", s_sList.asList[x].szLink );
      DBG_PRINTF( "TimesShared   = [%s]", s_sList.asList[x].szTimesShared );
      DBG_PRINTF( "----------------------------------------" );
   }
#endif
   return NO_ERROR;
}

ERROR_CODE Database_GetUnsharedPost(BLOG_POST *psPost)
{
   // TODO: Find the oldest least shared post

   return FAILED;
}