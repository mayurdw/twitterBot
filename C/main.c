/*
* Author: Mayur Wadhwani
* Date: 30th Novemeber 2019
*/

#include "Utils.h"
#include "config.h"
#include "CurlWrapper.h"

#define BLOG_FEED_URL           "https://itsmayurremember.wordpress.com/feed"

// Static Functions

int main()
{
    // Check against database
    // Select a new file
    RETURN_ON_FAIL( ReadConfig() );
    if( IsNewFileRequired() )
    {
        printf( "Downloading new feed file" );
        RETURN_ON_FAIL( DownloadFeedFile( BLOG_FEED_URL ) );
        UpdateConfig( CONFIG_DAYS_UNTIL_UPDATE, DAYS_UNTIL_NEXT_UPDATE );
    }

    return(0);
}