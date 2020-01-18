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
#include <libxml/xmlreader.h>

#define MAX_FILENAME_LEN        16
#define BLOG_FEED_URL           "https://itsmayurremember.wordpress.com/feed"
#define FILENAME_KEY            "FEEDFILE"
#define CONFIG_FILENAME         "config.xml"
#define DAYS_UNTIL_NEXT_UPDATE  "14"

typedef enum
{
    NO_ERROR = 0,
    INVALID_ARG,
    FILE_ERROR,
    CONFIG_VALIDATION_ERROR,
} ERROR_CODE;

typedef struct 
{
  char szFileName[MAX_FILENAME_LEN+1];
  FILE *psStream;
} RSS_FILE_STREAM;

typedef enum 
{
  CONFIG_CURRENT_FILENAME = 0,
  CONFIG_DAYS_UNTIL_UPDATE,

  CONFIG_LAST
}CONFIG_KEYS;

static const char* s_pszFileFormat = "%04i%02i%02i.xml";
static char s_aszConfigs[CONFIG_LAST][MAX_FILENAME_LEN+1] = {0,};
// Static Functions
static ERROR_CODE GenerateFileName( char *pszFileName, uint32_t ulBufferSize );
static size_t writeStreamToFile(void *pvBuffer, size_t iSize, size_t iNMemb, void *pvStream);
static ERROR_CODE DownloadFeedFile( const char *pszURL );
static ERROR_CODE InitConfig(void);
static void DebugConfig( void );

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

  static const char* s_apszConfigList[CONFIG_LAST] = 
  {
    "currentFilename",
    "daysToFileUpdate"
  };
  
static ERROR_CODE InitConfig(void)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL;/* node pointers */
    char buff[256] = {0,};
    int i = 0, j = 0;
    char szFilename[MAX_FILENAME_LEN+1] = {0,};

    GenerateFileName(szFilename, sizeof(szFilename));

    LIBXML_TEST_VERSION;

    /* 
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(doc, root_node);

    /* 
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node. 
     */
    xmlNewChild(root_node, NULL, BAD_CAST s_apszConfigList[0], BAD_CAST szFilename );
    strcpy( s_aszConfigs[0], szFilename );
    xmlNewChild(root_node,NULL, BAD_CAST s_apszConfigList[1], BAD_CAST DAYS_UNTIL_NEXT_UPDATE );
    strcpy( s_aszConfigs[1], DAYS_UNTIL_NEXT_UPDATE);
    /* 
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc(CONFIG_FILENAME, doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return NO_ERROR;
}

ERROR_CODE ReadXml( const char *pszFilename, const char **ppaszList, uint32_t ulArraySize )
{
  ERROR_CODE eRet = NO_ERROR;
  xmlTextReaderPtr pReader = NULL;
  CONFIG_KEYS x = 0;

  if( pszFilename == NULL || ppaszList == NULL || ulArraySize == 0 )
  {
    return INVALID_ARG;
  }

  pReader = xmlReaderForFile(pszFilename, NULL, 0 );
  if (pReader != NULL) 
  {
    while (xmlTextReaderRead(pReader) == 1 && x < ulArraySize ) 
    {
        const xmlChar *pName = xmlTextReaderConstName(pReader);

        if( pName != NULL && ( strcmp( ppaszList[x], pName) == 0 ) )
        {
          xmlTextReaderRead(pReader);
          const xmlChar *pValue = xmlTextReaderConstValue(pReader);
          strcpy( s_aszConfigs[x], pValue );
          x++;
        }        
    }
    
    xmlFreeTextReader(pReader);
  } 
  else 
  {
    eRet = FILE_ERROR;
  }

  // /*
  //  * Cleanup function for the XML library.
  //  */
  xmlCleanupParser();
  // /*
  //  * this is to debug memory for regression tests
  //  */
  xmlMemoryDump();

  return eRet;
}

ERROR_CODE ReadConfig( void )
{
  return ReadXml( CONFIG_FILENAME, s_apszConfigList, CONFIG_LAST );
}

static void DebugConfig( void )
{
  printf( "%s: Debugging config\n", __func__ );
  for( CONFIG_KEYS x = 0; x < CONFIG_LAST; x++ )
  {
    printf( "s_aszConfigs[%d] = %s\n", x, s_aszConfigs[x] );
  }
}

int main() 
{
  ERROR_CODE eRet = ReadConfig();
  if( eRet != NO_ERROR )
  {
    printf( "Error %d with reading config file\n", eRet );
    InitConfig();
  }
  DebugConfig();
    
  return(0);
}