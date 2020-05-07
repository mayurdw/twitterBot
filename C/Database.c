/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

// Static variables
static BLOG_POST s_asList[200] = { 0, };
static xmlWrapper *psTextReader = _null_;

// Static functions
static ERROR_CODE ReadDatabaseFile( void );

ERROR_CODE ReadFeedXmlFile( void )
{
   static XML_ITEMS s_sItemList = { "item", { "title ", "link", "category", "category", "category", "description"} };
    char szFilename[MAX_FILENAME_LEN + 1] = { 0, };
    UTIL_STR_ARRAY sConfigValue = { 0, };
    const UTIL_STR_ARRAY sKeys = { "title", "link" };

    RETURN_ON_FAIL( GetConfig( CONFIG_CURRENT_FILENAME, szFilename, sizeof( szFilename ) ) );
    printf( "filename = %s\n", szFilename );

    RETURN_ON_FAIL( ReadXml( szFilename, &sKeys, &sConfigValue ) );

    return NO_ERROR;
}

ERROR_CODE ReadDatabaseFile( void )
{
   return NO_ERROR;
}
