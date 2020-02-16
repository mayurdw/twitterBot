/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/
#include <curl/curl.h>
#include "CurlWrapper.h"

// Static Functions
static size_t writeStreamToFile( void * pvBuffer, size_t iSize, size_t iNMemb, void * pvStream );

typedef struct
{
    char szFileName[MAX_FILENAME_LEN + 1];
    FILE * psStream;
} RSS_FILE_STREAM;

static size_t writeStreamToFile( void * pvBuffer, size_t iSize, size_t iNMemb, void * pvStream )
{
    RSS_FILE_STREAM * psOutStream = ( RSS_FILE_STREAM * )pvStream;
    if( !psOutStream->psStream )
    {
        /* open file for writing */
        psOutStream->psStream = fopen( psOutStream->szFileName, "w+" );
        if( !psOutStream->psStream )
            return -1; /* failure, can't open file to write */
    }
    return fwrite( pvBuffer, iSize, iNMemb, psOutStream->psStream );
}


ERROR_CODE DownloadFeedFile( const char * pszURL )
{
    CURLM * psCurl = NULL;
    CURLcode resCode = CURLE_OK;
    RSS_FILE_STREAM sFileStream = { 0, };

    GenerateFileName( sFileStream.szFileName, sizeof( sFileStream.szFileName ) );

    curl_global_init( CURL_GLOBAL_ALL );
    psCurl = curl_easy_init();

    if( psCurl )
    {
        curl_easy_setopt( psCurl, CURLOPT_URL, pszURL );
        curl_easy_setopt( psCurl, CURLOPT_FOLLOWLOCATION, 1 );
        curl_easy_setopt( psCurl, CURLOPT_WRITEFUNCTION, writeStreamToFile );
        curl_easy_setopt( psCurl, CURLOPT_WRITEDATA, &sFileStream );
        resCode = curl_easy_perform( psCurl );
        curl_easy_cleanup( psCurl );
    }

    if( sFileStream.psStream )
    {
        fclose( sFileStream.psStream );
    }

    curl_global_cleanup();

    return NO_ERROR;
}
