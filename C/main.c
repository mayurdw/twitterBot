/*
* Author: Mayur Wadhwani
* Date: 30th Novemeber 2019
*/

#include "Utils.h"
#include "config.h"
#include "CurlWrapper.h"
#include "Database.h"

#define BLOG_FEED_URL           "https://itsmayurremember.wordpress.com/feed"
#define DAYS_UNTIL_NEXT_UPDATE  "14"

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


int main()
{
   // Config init
   RETURN_ON_FAIL( ReadConfig() );
   if( IsNewFileRequired() )
   {
      printf( "Downloading new feed file\n" );
      RETURN_ON_FAIL( DownloadFeedFile( BLOG_FEED_URL ) );
      RETURN_ON_FAIL( Config_SetDaysUntilUpdate( DAYS_UNTIL_NEXT_UPDATE ) );
   }

   RETURN_ON_FAIL( ReadFeedXmlFile() );

   return(0);
}