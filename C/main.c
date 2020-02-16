/*
* Author: Mayur Wadhwani
* Date: 30th Novemeber 2019
*/

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <curl/curl.h>
#include "Utils.h"
#include "config.h"

#define BLOG_FEED_URL           "https://itsmayurremember.wordpress.com/feed"
#define FILENAME_KEY            "FEEDFILE"
#define DAYS_UNTIL_NEXT_UPDATE  "14"

typedef struct
{
    char szFileName[MAX_FILENAME_LEN + 1];
    FILE* psStream;
} RSS_FILE_STREAM;

// Static Functions
static size_t writeStreamToFile(void* pvBuffer, size_t iSize, size_t iNMemb, void* pvStream);
static ERROR_CODE DownloadFeedFile(const char* pszURL);
static ERROR_CODE InitConfig(void);

static size_t writeStreamToFile(void* pvBuffer, size_t iSize, size_t iNMemb, void* pvStream)
{
    RSS_FILE_STREAM* psOutStream = (RSS_FILE_STREAM*)pvStream;
    if (!psOutStream->psStream) {
        /* open file for writing */
        psOutStream->psStream = fopen(psOutStream->szFileName, "w+");
        if (!psOutStream->psStream)
            return -1; /* failure, can't open file to write */
    }
    return fwrite(pvBuffer, iSize, iNMemb, psOutStream->psStream);
}

static ERROR_CODE DownloadFeedFile(const char* pszURL)
{
    CURLM* psCurl = NULL;
    CURLcode resCode = CURLE_OK;
    RSS_FILE_STREAM sFileStream = { 0, };

    GenerateFileName(sFileStream.szFileName, sizeof(sFileStream.szFileName));

    curl_global_init(CURL_GLOBAL_ALL);
    psCurl = curl_easy_init();

    if (psCurl)
    {
        curl_easy_setopt(psCurl, CURLOPT_URL, pszURL);
        curl_easy_setopt(psCurl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(psCurl, CURLOPT_WRITEFUNCTION, writeStreamToFile);
        curl_easy_setopt(psCurl, CURLOPT_WRITEDATA, &sFileStream);
        resCode = curl_easy_perform(psCurl);
        curl_easy_cleanup(psCurl);
    }

    if (sFileStream.psStream)
    {
        fclose(sFileStream.psStream);
    }

    curl_global_cleanup();

    return NO_ERROR;
}

int main()
{
    // Download feed file
    // Check against database
    // Select a new file
    ReadConfig();
    if( IsNewFileRequired() )
    {
        printf( "Downloading new feed file" );
        DownloadFeedFile( BLOG_FEED_URL );
    }



    return(0);
}