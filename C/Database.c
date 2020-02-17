/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

ERROR_CODE ReadFeedXmlFile( void )
{
    char szFilename[MAX_FILENAME_LEN + 1] = { 0, };
    const UTIL_STR_ARRAY sConfigKeys = { "TEST", "TEST" };
    UTIL_STR_ARRAY sConfigValue = { 0, };

    RETURN_ON_FAIL( GetConfig( CONFIG_CURRENT_FILENAME, szFilename, sizeof( szFilename ) ) );
    printf( "filename = %s", szFilename );

    return NO_ERROR;
}
