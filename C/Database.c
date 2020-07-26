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

bool Database_IsUniquePost( const BLOG_POST *psPost )
{
   bool bIsUnique = false;

   if( _null_ == psPost )
   {
#if DATABASE_DEBUG
      DBG_PRINTF( "psPost is NULL, returned false" );
#endif
   }
   else
   {
      uint32_t ulCount = atol( s_sList.szPostCount );
      if( ulCount == 0 )
      {
         bIsUnique = true;
      }
      else
      {
         bool bFound = false;

         for( uint32_t x = 0; ( x < ulCount && !bFound ); x++ )
         {
            bFound = memcmp( psPost, &s_sList.asList[x], sizeof( BLOG_POST ) ) == 0;
         }

         bIsUnique = !bFound;
      }
      
   }
   

   return bIsUnique; 
}

ERROR_CODE Database_AddNewItem( const BLOG_POST *psPost )
{
   const uint32_t ulCount = atol( s_sList.szPostCount );

   RETURN_ON_NULL( psPost );
   UTIL_ASSERT( ( strlen( psPost->szLink ) > 0 && strlen( psPost->szTitle ) > 0 ), INVALID_ARG );
   UTIL_ASSERT( ( ARRAY_COUNT( s_sList.asList ) > ( ulCount + 1 ) ), OVERFLOW );

   if( ulCount == 0 )
   {
      memcpy( &s_sList.asList[0], psPost, sizeof( BLOG_POST ) );
   }
   else
   {
      const uint32_t ulCopySize = sizeof( s_sList.asList ) - sizeof ( BLOG_POST );

      memmove( &s_sList.asList[1], &s_sList.asList[0], ulCopySize );
      memcpy( &s_sList.asList[0], psPost, sizeof( BLOG_POST ) );
   }

   snprintf( s_sList.szPostCount, sizeof( s_sList.szPostCount ), "%u", ulCount + 1 );
   
   return NO_ERROR;
}


////////////////////////////////////////////////////////////////////////////


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
   RETURN_ON_FAIL( Database_IsUniquePost( _null_ ) == false ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( Database_AddNewItem( _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) == INVALID_ARG ? NO_ERROR : FAILED );

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

static ERROR_CODE Database_OldestPostTest()
{
   BLOG_POST sPost = {0, };

   PRINTF_TEST( "Should return oldest post in the list" );
   memset( &s_sList, 0, sizeof( s_sList ) );
   
   Strcpy_safe( s_sList.asList[0].szLink, "LINK 1", sizeof( s_sList.asList[0].szLink ) );
   Strcpy_safe( s_sList.asList[0].szTitle, "TITLE 1", sizeof( s_sList.asList[0].szTitle ) );
   Strcpy_safe( s_sList.asList[0].szTimesShared, "10", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[1].szLink, "LINK 2", sizeof( s_sList.asList[1].szLink ) );
   Strcpy_safe( s_sList.asList[1].szTitle, "TITLE 2", sizeof( s_sList.asList[1].szTitle ) );
   Strcpy_safe( s_sList.asList[1].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[2].szLink, "LINK 3", sizeof( s_sList.asList[2].szLink ) );
   Strcpy_safe( s_sList.asList[2].szTitle, "TITLE 3", sizeof( s_sList.asList[2].szTitle ) );
   Strcpy_safe( s_sList.asList[2].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.szPostCount, "3", sizeof( s_sList.szPostCount ) );

   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( &sPost ) );

   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[2], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
}

static ERROR_CODE Database_IsUniqueSimpleTest( void )
{
   BLOG_POST sPost = { "Unique Title", "Unique Link", "0" };
   bool bRet = false;

   PRINTF_TEST( "Simple unique test" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   bRet = Database_IsUniquePost( &sPost );
   RETURN_ON_FAIL( bRet ? NO_ERROR : FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );

   return NO_ERROR;
}

static ERROR_CODE Database_IsUniqueFilledDatabase( void )
{
   bool bRet = false;
   BLOG_POST sPost = { "UNIQUE TITLE", "UNIQUE LINK", "0" };

   PRINTF_TEST( "Filled Database Unique test" );
   memset( &s_sList, 0, sizeof( s_sList ) );
   Strcpy_safe( s_sList.asList[0].szLink, "LINK 1", sizeof( s_sList.asList[0].szLink ) );
   Strcpy_safe( s_sList.asList[0].szTitle, "TITLE 1", sizeof( s_sList.asList[0].szTitle ) );
   Strcpy_safe( s_sList.asList[0].szTimesShared, "10", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[1].szLink, "LINK 2", sizeof( s_sList.asList[1].szLink ) );
   Strcpy_safe( s_sList.asList[1].szTitle, "TITLE 2", sizeof( s_sList.asList[1].szTitle ) );
   Strcpy_safe( s_sList.asList[1].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[2].szLink, "LINK 3", sizeof( s_sList.asList[2].szLink ) );
   Strcpy_safe( s_sList.asList[2].szTitle, "TITLE 3", sizeof( s_sList.asList[2].szTitle ) );
   Strcpy_safe( s_sList.asList[2].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.szPostCount, "3", sizeof( s_sList.szPostCount ) );

   memset( &s_sList, 0, sizeof( s_sList ) );
   bRet = Database_IsUniquePost( &sPost );
   RETURN_ON_FAIL( bRet ? NO_ERROR : FAILED );

   return NO_ERROR;
}

static ERROR_CODE Database_IsNotUniqueFilledDatabase( void )
{
   bool bRet = false;
   BLOG_POST sPost = { "TITLE 2", "LINK 2", "1" };

   PRINTF_TEST( "Filled Database Not Unique test" );
   memset( &s_sList, 0, sizeof( s_sList ) );
   Strcpy_safe( s_sList.asList[0].szLink, "LINK 1", sizeof( s_sList.asList[0].szLink ) );
   Strcpy_safe( s_sList.asList[0].szTitle, "TITLE 1", sizeof( s_sList.asList[0].szTitle ) );
   Strcpy_safe( s_sList.asList[0].szTimesShared, "10", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[1].szLink, "LINK 2", sizeof( s_sList.asList[1].szLink ) );
   Strcpy_safe( s_sList.asList[1].szTitle, "TITLE 2", sizeof( s_sList.asList[1].szTitle ) );
   Strcpy_safe( s_sList.asList[1].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[2].szLink, "LINK 3", sizeof( s_sList.asList[2].szLink ) );
   Strcpy_safe( s_sList.asList[2].szTitle, "TITLE 3", sizeof( s_sList.asList[2].szTitle ) );
   Strcpy_safe( s_sList.asList[2].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.szPostCount, "3", sizeof( s_sList.szPostCount ) );

   bRet = Database_IsUniquePost( &sPost );
   
   memset( &s_sList, 0, sizeof( s_sList ) );
   RETURN_ON_FAIL( !bRet ? NO_ERROR : FAILED );

   return NO_ERROR;
}

static ERROR_CODE Database_AddSimpleItem( void )
{
   BLOG_POST sPost = { "NEW TITLE", "NEW LINK", "0" };

   PRINTF_TEST( "Testing adding item" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) );
   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[0], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( s_sList.szPostCount, "1" ) == 0 ? NO_ERROR : FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
   return NO_ERROR;
}

static ERROR_CODE Database_AddItemToFilledDatabase( void )
{
   BLOG_POST sPost = { "NEW TITLE", "NEW LINK", "0" };

   PRINTF_TEST( "Testing adding item on a filled database" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   Strcpy_safe( s_sList.asList[0].szLink, "LINK 1", sizeof( s_sList.asList[0].szLink ) );
   Strcpy_safe( s_sList.asList[0].szTitle, "TITLE 1", sizeof( s_sList.asList[0].szTitle ) );
   Strcpy_safe( s_sList.asList[0].szTimesShared, "10", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[1].szLink, "LINK 2", sizeof( s_sList.asList[1].szLink ) );
   Strcpy_safe( s_sList.asList[1].szTitle, "TITLE 2", sizeof( s_sList.asList[1].szTitle ) );
   Strcpy_safe( s_sList.asList[1].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.asList[2].szLink, "LINK 3", sizeof( s_sList.asList[2].szLink ) );
   Strcpy_safe( s_sList.asList[2].szTitle, "TITLE 3", sizeof( s_sList.asList[2].szTitle ) );
   Strcpy_safe( s_sList.asList[2].szTimesShared, "1", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.szPostCount, "3", sizeof( s_sList.szPostCount ) );

   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) );
   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[0], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( s_sList.szPostCount, "4" ) == 0 ? NO_ERROR : FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
   return NO_ERROR;
}

static ERROR_CODE Database_AddItemDatabaseFull() 
{
   BLOG_POST sPost = { "UNIQUE TITLE", "UNIQUE TEST", "0" };
   char szTemp[32 + 1] = { 0, };
   const uint32_t ulCount = ARRAY_COUNT( s_sList.asList );

   PRINTF_TEST( "Add Item: Database is full" );

   memset( &s_sList, 0, sizeof( s_sList ) );

   for( uint32_t x = 0; x < ulCount; x++ )
   {
      snprintf( szTemp, sizeof( szTemp ), "TITLE %u", x );
      Strcpy_safe( s_sList.asList[x].szTitle, szTemp, sizeof( s_sList.asList[x].szTitle ) );
      snprintf( szTemp, sizeof( szTemp ), "LINK %u", x );
      Strcpy_safe( s_sList.asList[x].szLink, szTemp, sizeof( s_sList.asList[x].szLink ) );
   }
   snprintf( szTemp, sizeof( szTemp ), "%u", ulCount );
   Strcpy_safe( s_sList.szPostCount, szTemp, sizeof( s_sList.szPostCount ) );

   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) == OVERFLOW ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( s_sList.szPostCount, szTemp ) == 0 ? NO_ERROR : FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
   
   return NO_ERROR;
}

ERROR_CODE Database_Tests( void )
{
   memset( &s_sList, 0, sizeof( s_sList ) );
   
   RETURN_ON_FAIL( Database_SanityTest() );
   RETURN_ON_FAIL( Database_SimpleComparison() );
   RETURN_ON_FAIL( Database_OldestPostTest() );
   RETURN_ON_FAIL( Database_IsUniqueSimpleTest() );
   RETURN_ON_FAIL( Database_IsUniqueFilledDatabase() );
   RETURN_ON_FAIL( Database_IsNotUniqueFilledDatabase() );
   RETURN_ON_FAIL( Database_AddSimpleItem() );
   RETURN_ON_FAIL( Database_AddItemToFilledDatabase() );
   RETURN_ON_FAIL( Database_AddItemDatabaseFull() );

   memset( &s_sList, 0, sizeof( s_sList ) );
   DBG_PRINTF( "------------- %s: [%u] Tests passed -------------", __func__, s_ulTestCount );

   return NO_ERROR;
}
