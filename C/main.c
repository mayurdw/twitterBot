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

#define MAX_FILENAME_LEN    16
#define BLOG_FEED_URL       "https://itsmayurremember.wordpress.com/feed"
#define FILENAME_EXT        ".xml"
#define FILENAME_KEY        "FEEDFILE"
#define CONFIG_FILENAME     "config.xml"

typedef enum
{
    NO_ERROR = 0,
    INVALID_ARG,
    FILE_ERROR,
    CONFIG_VALIDATION_ERROR,
    XML_PARSING_ERROR,
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
static ERROR_CODE InitConfig(void);

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

static ERROR_CODE InitConfig(void)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL;/* node pointers */
    char buff[256] = {0,};
    int i = 0, j = 0;

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
    xmlNewChild(root_node, NULL, BAD_CAST "currentFilename",
                BAD_CAST "26122019.xml");
    xmlNewChild(root_node,NULL, BAD_CAST "daysToFileUpdate", BAD_CAST "14");

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

/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
static void
processNode(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
	name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);

    printf("%d %d %s %d %d", 
	    xmlTextReaderDepth(reader),
	    xmlTextReaderNodeType(reader),
	    name,
	    xmlTextReaderIsEmptyElement(reader),
	    xmlTextReaderHasValue(reader));
    if (value == NULL)
	printf("\n");
    else {
        if (xmlStrlen(value) > 40)
            printf(" %.40s...\n", value);
        else
	    printf(" %s\n", value);
    }
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse, validate and print information about an XML file.
 */
static void
streamFile(const char *filename) {
    
}

ERROR_CODE ReadConfig( void )
{
  ERROR_CODE eRet = NO_ERROR;
  xmlTextReaderPtr pReader = NULL;
  int iRet = 0;

  /*
    * Pass some special parsing options to activate DTD attribute defaulting,
    * entities substitution and DTD validation
  */
  pReader = xmlReaderForFile(CONFIG_FILENAME, NULL,
                XML_PARSE_RECOVER ); /* validate with the DTD */

  if (pReader != NULL) 
  {
    iRet = xmlTextReaderRead(pReader);
    while (iRet == 1) 
    {
        processNode(pReader);
        iRet = xmlTextReaderRead(pReader);
    }
  
  /*
	 * Once the document has been fully parsed check the validation results
	 */
  if (xmlTextReaderIsValid(pReader) != 1) 
  {
    eRet = CONFIG_VALIDATION_ERROR;
	}
  
  xmlFreeTextReader(pReader);
  if (iRet != 0) {
    eRet = XML_PARSING_ERROR;
  }
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

int main() 
{

    if( ReadConfig() != NO_ERROR )
    {
      printf( "Error with reading config file\n");
      InitConfig();
    }
    
    return(0);
}