/*
* Author: Mayur Wadhwani
* Date: 30th Novemeber 2019
*/

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <curl/curl.h>

typedef enum
{
    NO_ERROR = 0,
} ERROR_CODE;

typedef struct 
{
  const char *pszFilename;
  FILE *psStream;
} RSS_FILE_STREAM;
 
static size_t writeStreamToFile(void *pvBuffer, size_t iSize, size_t iNMemb, void *pvStream)
{
  RSS_FILE_STREAM *psOutStream = (RSS_FILE_STREAM *)pvStream;
  if(!psOutStream->psStream) {
    /* open file for writing */ 
    psOutStream->psStream = fopen(psOutStream->pszFilename, "wb");
    if(!psOutStream->psStream)
      return -1; /* failure, can't open file to write */ 
  }
  return fwrite(pvBuffer, iSize, iNMemb, psOutStream->psStream);
}

ERROR_CODE DownloadFeedFile( const char *pszURL )
{
    CURLM *psCurl = NULL;
    CURLcode resCode = CURLE_OK;
    RSS_FILE_STREAM sFileStream = { "Test.txt", NULL };

    curl_global_init(CURL_GLOBAL_ALL);
    psCurl = curl_easy_init();

    if( psCurl )
    {
        curl_easy_setopt(psCurl, CURLOPT_URL, pszURL);
        curl_easy_setopt(psCurl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(psCurl,CURLOPT_WRITEFUNCTION, writeStreamToFile);
        curl_easy_setopt(psCurl,CURLOPT_WRITEDATA, &sFileStream);
        resCode = curl_easy_perform(psCurl);
        curl_easy_cleanup(psCurl);
    }

    if( sFileStream.psStream )
    {
        fclose(sFileStream.psStream);
    }

    curl_global_cleanup();
    
    return NO_ERROR;
}

bool FeedFileExists( const char *pszFileName )
{
    return false;
}

ERROR_CODE GenerateFileName( char *pszFileName, uint32_t ulBufferSize )
{
    return NO_ERROR;
}

int main( )
{
    DownloadFeedFile("https://itsmayurremember.wordpress.com/feed");
    return 0;
}