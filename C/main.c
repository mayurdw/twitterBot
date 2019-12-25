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
#include <string.h>
#include <dirent.h>
#include <libxml/parser.h>

#define MAX_FILENAME_LEN    16
#define BLOG_FEED_URL       "https://itsmayurremember.wordpress.com/feed"
#define FILENAME_EXT        ".xml"
#define CONFIG_FILENAME     "CONFIG"
#define FILENAME_KEY        "FEEDFILE"

typedef enum
{
    NO_ERROR = 0,
    INVALID_ARG,
    FILE_ERROR,
} ERROR_CODE;

typedef struct 
{
  char szFileName[MAX_FILENAME_LEN+1];
  FILE *psStream;
} RSS_FILE_STREAM;

static const char* s_pszFileFormat = "%04i%02i%02i" FILENAME_EXT;
// Static Functions
static ERROR_CODE GenerateFileName( char *pszFileName, uint32_t ulBufferSize );
static size_t writeStreamToFile(void *pvBuffer, size_t iSize, size_t iNMemb, void *pvStream);
static ERROR_CODE DownloadFeedFile( const char *pszURL );
static bool FeedFileExists( void );

static size_t writeStreamToFile(void *pvBuffer, size_t iSize, size_t iNMemb, void *pvStream)
{
  RSS_FILE_STREAM *psOutStream = (RSS_FILE_STREAM *)pvStream;
  if(!psOutStream->psStream) {
    /* open file for writing */ 
    psOutStream->psStream = fopen(psOutStream->szFileName, "wb");
    if(!psOutStream->psStream)
      return -1; /* failure, can't open file to write */ 
  }
  return fwrite(pvBuffer, iSize, iNMemb, psOutStream->psStream);
}

static ERROR_CODE DownloadFeedFile( const char *pszURL )
{
    CURLM *psCurl = NULL;
    CURLcode resCode = CURLE_OK;
    RSS_FILE_STREAM sFileStream = {0, };

    GenerateFileName(sFileStream.szFileName, sizeof( sFileStream.szFileName));

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

static bool FeedFileExists( void )
{
    DIR *psDir = opendir("./");
    bool bRet = false;

    if( psDir != NULL )
    {
       struct dirent *psCurrentFile = NULL;

       while( !bRet && ( psCurrentFile = readdir(psDir) ) != NULL )
       {
           bRet = ( strstr( psCurrentFile->d_name, FILENAME_EXT) != NULL);
       }

    }

    return bRet;
}

static ERROR_CODE GenerateFileName( char *pszFileName, uint32_t ulBufferSize )
{
    time_t sTime = {0,};
    struct tm *psTimeNow = NULL; 
    if( !pszFileName || ulBufferSize < MAX_FILENAME_LEN)
    return INVALID_ARG;

    memset(pszFileName, 0, ulBufferSize);
    time(&sTime);
    psTimeNow = localtime(&sTime);

    snprintf(pszFileName, ulBufferSize, s_pszFileFormat, psTimeNow->tm_year + 1900, psTimeNow->tm_mon + 1, psTimeNow->tm_mday );

    return NO_ERROR;
}

static ERROR_CODE SetLastFeedFileName( const char *pszFileName )
{
    return NO_ERROR;
}

static ERROR_CODE GetLastFeedFilename( char *pszFilename, uint32_t ulFilename)
{
    return NO_ERROR;
}
 
int is_leaf(xmlNode * node)
{
  xmlNode * child = node->children;
  while(child)
  {
    if(child->type == XML_ELEMENT_NODE) return 0;
 
    child = child->next;
  }
 
  return 1;
}
 
void print_xml(xmlNode * node, int indent_len)
{
    while(node)
    {
        if(node->type == XML_ELEMENT_NODE)
        {
          printf("%*c%s:%s\n", indent_len*2, '-', node->name, is_leaf(node)?xmlNodeGetContent(node):xmlGetProp(node, "id"));
        }
        print_xml(node->children, indent_len + 1);
        node = node->next;
    }
}

int main( )
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    char szFilename[MAX_FILENAME_LEN+1]={0,};

    DownloadFeedFile( BLOG_FEED_URL );
    GenerateFileName(szFilename, sizeof( szFilename ));

    doc = xmlReadFile(szFilename, NULL, 0);

    if (doc == NULL) {
    printf("Could not parse the XML file");
    }

    root_element = xmlDocGetRootElement(doc);

    print_xml(root_element, 1);

    xmlFreeDoc(doc);

    xmlCleanupParser();
    return 0;
}