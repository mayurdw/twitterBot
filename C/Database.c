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

ERROR_CODE Database_GetOldestLeastSharedPost(BLOG_POST * psPost)
{
   const uint32_t ulPostCount = atol( s_sList.szPostCount );
   uint32_t ulOldestCount = UINT32_MAX, ulIndexFound = UINT32_MAX;
   ERROR_CODE eRet = NO_ERROR;

   // TODO: Find the oldest least shared post
   RETURN_ON_NULL( psPost );
   memset( psPost, 0, sizeof( BLOG_POST ) );

   if( ulPostCount == 0 || ( ulPostCount > ARRAY_COUNT( s_sList.asList ) ) )
      return NOT_FOUND;

   for( uint32_t x = 0; x < ulPostCount; x++ )
   {
      uint32_t ulTemp = atol( s_sList.asList[x].szTimesShared );
      if( ulOldestCount >= ulTemp )
      {
         // By making it >=, we shall make sure we use the oldest & least
         ulOldestCount = ulTemp;
         ulIndexFound = x;
      }
   }

   if( ulIndexFound < ARRAY_COUNT( s_sList.asList ) )
   {
      *psPost = s_sList.asList[ulIndexFound];
   }
   else
   {
      DBG_PRINTF( "Unable to find a valid index, it is [%u]", ulIndexFound );
      eRet = NOT_FOUND;
   }

   return eRet;
}


static uint32_t s_ulTestCount = 0;

#if DEBUG_DATABASE
#define PRINTF_TEST(string) ( DBG_PRINTF( "----- %s | Test Count: %u -----", string, s_ulTestCount++ ) ) 
#else
#define PRINTF_TEST(string) ( s_ulTestCount++ )
#endif


static ERROR_CODE Database_SanityTest( void )
{
   BLOG_POST sPost = { 0, };
   PRINTF_TEST( "Basic Sanity Testing" );
   
   memset( &s_sList, 0, sizeof( s_sList ) );
   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( &sPost ) == NOT_FOUND ? NO_ERROR : FAILED );

   return NO_ERROR;
}

static ERROR_CODE Database_SimpleComparison( void )
{
   BLOG_POST sPost = { 0, };

   PRINTF_TEST( "Simple Comparison between two posts" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   Strcpy_safe( s_sList.asList[0].szLink, "LINK 1", sizeof( s_sList.asList[0].szLink ) );
   Strcpy_safe( s_sList.asList[0].szTitle, "TITLE 1", sizeof( s_sList.asList[0].szTitle ) );
   Strcpy_safe( s_sList.asList[0].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[1].szLink, "LINK 2", sizeof( s_sList.asList[1].szLink ) );
   Strcpy_safe( s_sList.asList[1].szTitle, "TITLE 2", sizeof( s_sList.asList[1].szTitle ) );
   Strcpy_safe( s_sList.szPostCount, "2", sizeof( s_sList.szPostCount ) );

   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( &sPost ) );

   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[1], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : FAILED );

   return NO_ERROR;
}

ERROR_CODE Database_Tests( void )
{
   RETURN_ON_FAIL( Database_SanityTest() );
   RETURN_ON_FAIL( Database_SimpleComparison() );

   DBG_PRINTF( "------------- %s: [%u] Tests passed -------------", __func__, s_ulTestCount );

   return NO_ERROR;
}
