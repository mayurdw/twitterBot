/*
* Author: Mayur Wadhwani
* Date: 30th Novemeber 2019
*/

#include "Utils.h"
#include "config.h"
#include "CurlWrapper.h"
#include "Database.h"

#define BLOG_FEED_URL            ( "https://itsmayurremember.wordpress.com/feed" )
#define DAYS_UNTIL_NEXT_UPDATE   ( "14" )
#define PERFORM_TESTS            ( 0 )
// Static Functions

// Application flow:
// Check config files
// If config is absent, it will populate with default config
// It will check config to see if new RSS file needs to download, it will prompt curl to do so
// If new RSS file needs to be downloaded, it will ask the database to do it. 
// Check database
// Database will check if database file exists
// If database file exists, then great
// If not, it will create a database file with the existing RSS file
// It will give us a post


static ERROR_CODE readyPostForPublishing()
{
   BLOG_POST sPost = {0, };
   char szDaysUntilUpdate[2 + 1] = {0, };
   uint32_t ulDays = 0;

   RETURN_ON_FAIL( Database_GetOldestLeastSharedPost( &sPost ) );
   RETURN_ON_FAIL( Config_GetDaysUntilUpdate( szDaysUntilUpdate, sizeof( szDaysUntilUpdate ) ) );

   ulDays = atol( szDaysUntilUpdate );
   ulDays--;
   snprintf( szDaysUntilUpdate, sizeof( szDaysUntilUpdate ), "%u", ulDays );
   RETURN_ON_FAIL( Config_SetDaysUntilUpdate( szDaysUntilUpdate ) );
   RETURN_ON_FAIL( Database_UpdateTimesShared( &sPost ) );

   DBG_PRINTF( "Oldest Post is: " );
   DBG_PRINTF( "Title = [%s]", sPost.szTitle );
   DBG_PRINTF( "Link  = [%s]", sPost.szLink );

   DBG_PRINTF( "Tweet Text = " );
   DBG_PRINTF( "From the archives of my blog: '%s'\n\n%s", sPost.szTitle, sPost.szLink );
   

   return NO_ERROR;
}

int main()
{
   DBG_INIT();

#if PERFORM_TESTS
   RETURN_ON_FAIL( XmlTest() );
   RETURN_ON_FAIL( Database_Tests() );
#endif

   RETURN_ON_FAIL( Config_Init() );

   if( IsNewFileRequired() )
   {
      DBG_PRINTF( "Downloading new feed file" );
      RETURN_ON_FAIL( DownloadFeedFile( BLOG_FEED_URL ) );
      RETURN_ON_FAIL( Config_SetDaysUntilUpdate( DAYS_UNTIL_NEXT_UPDATE ) );
   }
   
   RETURN_ON_FAIL( Database_Init( ) );

   RETURN_ON_FAIL( readyPostForPublishing() );
   
   return( 0 );
}