/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

// Static variables
static BLOG_POST s_asList[200] = { 0, };
static xmlWrapperPtr psTextReader = _null_;

// Static functions
static ERROR_CODE ReadDatabaseFile( void );

ERROR_CODE ReadFeedXmlFile( void )
{
   const char *apszItemList[] = { "title", "link", "category", "category", "category", "description" };
    char szFilename[MAX_FILENAME_LEN + 1] = { 0, };
    char szTemp[4096 + 1] = { 0, };
    ERROR_CODE eRet = NO_ERROR;
    bool bFound = false;
    int x = 0, y = 0;


    RETURN_ON_FAIL( GetConfig( CONFIG_CURRENT_FILENAME, szFilename, sizeof( szFilename ) ) );

    RETURN_ON_FAIL( OpenXmlFile( &psTextReader, szFilename ) );

    while( y < 10 )
    {
       RETURN_ON_FAIL( FindElement( psTextReader, "item", &bFound ) );
       x = 0;
       while( bFound && x < 6 )
       {
          RETURN_ON_FAIL( ExtractDataFromElement( psTextReader, apszItemList[x++], szTemp, sizeof( szTemp ) ) );
          printf( "Data = %s\n", szTemp );
       }

       RETURN_ON_FAIL( FindElement( psTextReader, "item", &bFound ) );
       y++;
    }

    return NO_ERROR;
}

ERROR_CODE ReadDatabaseFile( void )
{
   return NO_ERROR;
}
