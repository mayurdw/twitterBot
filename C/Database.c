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
static ERROR_CODE Database_FindIndex( const BLOG_POST *psPost, int32_t *plIndex );
/* 
   Counts the number of valid posts in a given list. The count stops at the first invalid post
   @param (INPUT):      pasList  -> List of Blog Posts
   @param (INPUT):      ulArraySize -> Max Numberof items in pasList array
   @param (OUTPUT):     pulCount -> Number of valid posts
   @return              NO_ERROR -> Success
   @return              INVALID_PARAM -> One or more parameter is invalid
 */
static ERROR_CODE Database_CountPostsInList( const BLOG_POST *pasList, uint32_t ulArraySize, uint32_t *pulCount );

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

ERROR_CODE Database_RefreshDatabase( void )
{
   uint32_t ulRssFilePostCount = 0;
   DATABASE sTemp = {0, };
   char szRSSfeedFile[MAX_FILENAME_LEN + 1] = { 0, };
   bool bNeedToRewrite = false;

   memset( &s_sList, 0, sizeof( DATABASE ) );

   // Try to instantiate the database file from xml file
   RETURN_ON_FAIL( Config_GetRssFilename( szRSSfeedFile, sizeof( szRSSfeedFile ) ) );

   RETURN_ON_FAIL( ReadFeedXmlFile( szRSSfeedFile ) );
   
   sTemp = s_sList;
   RETURN_ON_FAIL( Database_CountPostsInList( sTemp.asList, ARRAY_COUNT( sTemp.asList ), &ulRssFilePostCount ) );

   RETURN_ON_FAIL( ReadDatabaseFile() );

   for( int x = ulRssFilePostCount - 1; x >=0 ; x )
   {
      if( Database_IsUniquePost( &sTemp.asList[x] ) )
      {
         RETURN_ON_FAIL( Database_AddNewItem( &sTemp.asList[x] ) );
         bNeedToRewrite = true;
      }
   }

   if( bNeedToRewrite )
   {
      RETURN_ON_FAIL( CreateDatabaseFile() );
   }

   return NO_ERROR;
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

   RETURN_ON_FAIL( Database_CountPostsInList( s_sList.asList, ARRAY_COUNT( s_sList.asList ), &x ) );
   
   if( x != 0 )
   {
      DBG_PRINTF( "Writing [%u] posts onto the database file", x );
      snprintf( s_sList.szPostCount, sizeof( s_sList.szPostCount ), "%u", x );
      eRet = xmlWrapperWriteFile( DATABASE_FILE, s_asPosts, ARRAY_COUNT( s_asPosts ), &s_sList );

      DebugDatabaseFile( );
   }
   else
   {
      DBG_PRINTF( "Database Count is 0" );
      eRet = INVALID_ARG;
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


/* 
   Returns the index of the post in the database if found
   @param[IN]   psPost  : Post which needs to be found
   @param[OUT] plIndex  : Index of the post if found, else -1
   @return NO_ERROR     : Success
   @return INVALID_ARG  : Invalid Post found
 */

static ERROR_CODE Database_FindIndex( const BLOG_POST *psPost, int32_t *plIndex )
{
   const uint32_t ulCount = atol( s_sList.szPostCount );

   RETURN_ON_NULL( psPost );
   RETURN_ON_NULL( plIndex );
   UTIL_ASSERT( ( strlen( psPost->szTitle ) > 0 && strlen( psPost->szLink ) > 0 ), INVALID_ARG );
   *plIndex = -1;

   if( ulCount > 0 )
   {
      bool bFound = false;
      uint32_t x = 0;

      for( ; ( !bFound && x < ulCount ); x++ )
      {
         bFound = ( strcmp( psPost->szLink, s_sList.asList[x].szLink ) == 0 && 
                    strcmp( psPost->szTitle, s_sList.asList[x].szTitle ) == 0 );
      }

      *plIndex = bFound ? ( int32_t )--x : -1;
   }

   return NO_ERROR;
}


bool Database_IsUniquePost( const BLOG_POST *psPost )
{
   bool bIsUnique = false;

   if( _null_ == psPost )
   {
#if DEBUG_DATABASE
      DBG_PRINTF( "psPost is NULL, returned false" );
#endif
   }
   else
   {
      int32_t lIndex = -1;

      RETURN_ON_FAIL( Database_FindIndex( psPost, &lIndex ) );
      bIsUnique = ( lIndex < 0 );
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

ERROR_CODE Database_UpdateTimesShared( const BLOG_POST *psPost )
{
   int32_t lIndex = -1;

   RETURN_ON_NULL( psPost );
   UTIL_ASSERT( ( strlen( psPost->szTitle ) > 0 && strlen( psPost->szLink ) > 0 ), INVALID_ARG );

   RETURN_ON_FAIL( Database_FindIndex( psPost, &lIndex ) );
   if( lIndex >= 0 )
   {
      uint32_t ulCurrentCount = atol( s_sList.asList[lIndex].szTimesShared );

      ulCurrentCount++;
      snprintf( s_sList.asList[lIndex].szTimesShared, sizeof( s_sList.asList[lIndex].szTimesShared), 
      "%u", ulCurrentCount );
   }

   RETURN_ON_FAIL( CreateDatabaseFile() );

   return NO_ERROR;
}

static ERROR_CODE Database_CountPostsInList( const BLOG_POST *pasList, uint32_t ulArraySize, uint32_t *pulCount )
{
   uint32_t x = 0;
   
   RETURN_ON_NULL( pasList );
   RETURN_ON_NULL( pulCount );
   UTIL_ASSERT( ulArraySize > 0, INVALID_ARG );

   for( x = ulArraySize; x > 0 && ( strlen( pasList[x - 1].szTitle ) == 0 ); x-- );

   *pulCount = x;

   return NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////


static uint32_t s_ulTestCount = 0;

#if DEBUG_DATABASE
#define PRINTF_TEST(string) ( DBG_PRINTF( "----- %s | Test Count: %u -----", string, s_ulTestCount++ ) ) 
#else
#define PRINTF_TEST(string) ( s_ulTestCount++ )
#endif


static ERROR_CODE Database_Test_Sanity( void )
{
   BLOG_POST sPost = { 0, };
   PRINTF_TEST( "Basic Sanity Testing" );
   
   memset( &s_sList, 0, sizeof( s_sList ) );
   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( _null_ ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( &sPost ) == NOT_FOUND ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_IsUniquePost( _null_ ) == false ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_AddNewItem( _null_ ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_UpdateTimesShared( _null_ ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_UpdateTimesShared( &sPost ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_CountPostsInList( _null_, 0, _null_ ) == INVALID_ARG ? NO_ERROR: TEST_FAILED );
   RETURN_ON_FAIL( Database_CountPostsInList( s_sList.asList, 0, _null_ ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( Database_CountPostsInList( s_sList.asList, ARRAY_COUNT( s_sList.asList ), _null_ ) == INVALID_ARG ? NO_ERROR : TEST_FAILED );
   
   return NO_ERROR;
}

static ERROR_CODE Database_Test_SimpleComparison( void )
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

   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[1], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : TEST_FAILED );

   return NO_ERROR;
}

static ERROR_CODE Database_Test_OldestPost()
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

   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[2], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : TEST_FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );

   return NO_ERROR;
}

static ERROR_CODE Database_Test_IsUniqueSimple( void )
{
   BLOG_POST sPost = { "Unique Title", "Unique Link", "0" };
   bool bRet = false;

   PRINTF_TEST( "Simple unique test" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   bRet = Database_IsUniquePost( &sPost );
   RETURN_ON_FAIL( bRet ? NO_ERROR : TEST_FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );

   return NO_ERROR;
}

static ERROR_CODE Database_Test_IsUniqueFilledDatabase( void )
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
   RETURN_ON_FAIL( bRet ? NO_ERROR : TEST_FAILED );

   return NO_ERROR;
}

static ERROR_CODE Database_Test_IsNotUniqueFilledDatabase( void )
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
   RETURN_ON_FAIL( !bRet ? NO_ERROR : TEST_FAILED );

   return NO_ERROR;
}

static ERROR_CODE Database_Test_AddSimpleItem( void )
{
   BLOG_POST sPost = { "NEW TITLE", "NEW LINK", "0" };

   PRINTF_TEST( "Testing adding item" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) );
   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[0], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( strcmp( s_sList.szPostCount, "1" ) == 0 ? NO_ERROR : TEST_FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
   return NO_ERROR;
}

static ERROR_CODE Database_Test_AddItemToFilledDatabase( void )
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
   RETURN_ON_FAIL( memcmp( &sPost, &s_sList.asList[0], sizeof( BLOG_POST ) ) == 0 ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( strcmp( s_sList.szPostCount, "4" ) == 0 ? NO_ERROR : TEST_FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
   return NO_ERROR;
}

static ERROR_CODE Database_Test_AddItemDatabaseFull() 
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

   RETURN_ON_FAIL( Database_AddNewItem( &sPost ) == OVERFLOW ? NO_ERROR : TEST_FAILED );
   RETURN_ON_FAIL( strcmp( s_sList.szPostCount, szTemp ) == 0 ? NO_ERROR : TEST_FAILED );

   memset( &s_sList, 0, sizeof( s_sList ) );
   
   return NO_ERROR;
}

static ERROR_CODE Database_Test_UpdatePostSimple() 
{
#define TITLE "TEST_TITLE"
#define LINK  "TEST LINK"
#define TIME  "1"

   BLOG_POST sPost = { TITLE, LINK, "0" };

   PRINTF_TEST( "Simple update post test" );
   memset( &s_sList, 0, sizeof( s_sList ) );

   Strcpy_safe( s_sList.asList[0].szTitle, TITLE, sizeof( s_sList.asList[0].szTitle ) );
   Strcpy_safe( s_sList.asList[0].szLink, LINK, sizeof( s_sList.asList[0].szLink ) );
   Strcpy_safe( s_sList.asList[0].szTimesShared, "0", sizeof( s_sList.asList[0].szTimesShared ) );
   Strcpy_safe( s_sList.szPostCount, "1", sizeof( s_sList.szPostCount ) );

   RETURN_ON_FAIL( Database_UpdateTimesShared( &sPost ) );

#if DEBUG_DATABASE
   DBG_PRINTF( "EXPECTED = " );
   DBG_PRINTF( "TITLE = [%s]", sPost.szTitle );
   DBG_PRINTF( "LINK  = [%s]", sPost.szLink );
   DBG_PRINTF( "TIMES = [%s]", sPost.szTimesShared );
   DBG_PRINTF( "ACTUAL = ")
   DBG_PRINTF( "TITLE = [%s]", s_sList.asList[0].szTitle );
   DBG_PRINTF( "LINK  = [%s]", s_sList.asList[0].szLink );
   DBG_PRINTF( "TIMES = [%s]", s_sList.asList[0].szTimesShared );
#endif

   RETURN_ON_FAIL( ( strcmp( s_sList.asList[0].szTimesShared, TIME ) == 0 ? NO_ERROR : TEST_FAILED ) );

#undef TITLE
#undef LINK
#undef TIME

   return NO_ERROR;
}

static ERROR_CODE Database_Test_CountList( void )
{
   BLOG_POST asList[20] = {0, };
   const uint32_t ulFillCount = 5;
   uint32_t ulReturnedCount = 0;

   PRINTF_TEST( "Simple Test to count the number of items" );

   for( int x = 0; x < ulFillCount; x++ )
   {
      snprintf( asList[x].szTitle, sizeof( asList[x].szTitle ), "Title %u", x );
   }

   RETURN_ON_FAIL( Database_CountPostsInList( asList, ARRAY_COUNT( asList ), &ulReturnedCount ) );
#if DEBUG_DATABASE
   DBG_PRINTF( "Returned Count = %u & Filled Count = %u", ulReturnedCount, ulFillCount );
#endif

   RETURN_ON_FAIL( ulReturnedCount == ulFillCount ? NO_ERROR : TEST_FAILED );

   return NO_ERROR;
}

ERROR_CODE Database_Tests( void )
{
   memset( &s_sList, 0, sizeof( s_sList ) );
   
   RETURN_ON_FAIL( Database_Test_Sanity() );
   RETURN_ON_FAIL( Database_Test_SimpleComparison() );
   RETURN_ON_FAIL( Database_Test_OldestPost() );
   RETURN_ON_FAIL( Database_Test_IsUniqueSimple() );
   RETURN_ON_FAIL( Database_Test_IsUniqueFilledDatabase() );
   RETURN_ON_FAIL( Database_Test_IsNotUniqueFilledDatabase() );
   RETURN_ON_FAIL( Database_Test_AddSimpleItem() );
   RETURN_ON_FAIL( Database_Test_AddItemToFilledDatabase() );
   RETURN_ON_FAIL( Database_Test_AddItemDatabaseFull() );
   RETURN_ON_FAIL( Database_Test_UpdatePostSimple() );
   RETURN_ON_FAIL( Database_Test_CountList() );

   memset( &s_sList, 0, sizeof( s_sList ) );
   DBG_PRINTF( "------------- %s: [%u] Tests passed -------------", __func__, s_ulTestCount );

   return NO_ERROR;
}
