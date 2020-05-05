/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

static xmlTextReaderPtr s_pReader = _null_;
static ERROR_CODE OpenXmlFile( const char* pszFilename );
static ERROR_CODE ExtractDataFromElement( const char* pszElementName, char* pszDataBuffer, uint32_t ulBufferLen );

static ERROR_CODE OpenXmlFile( const char* pszFilename )
{
   RETURN_ON_NULL( pszFilename );
   UTIL_ASSERT( strlen( pszFilename ) > 0, INVALID_ARG );

   if( s_pReader != _null_ )
   {
      // Close file, free memory etc.
      s_pReader = _null_;
   }

   s_pReader = xmlReaderForFile( pszFilename, _null_, 0 );

   return NO_ERROR;
}

static ERROR_CODE ExtractDataFromElement( const char* pszElementName, char* pszDataBuffer, uint32_t ulBufferLen )
{
   bool bFound = FALSE;
   xmlChar* pszValue = _null_;

   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( pszDataBuffer );
   UTIL_ASSERT( strlen( pszElementName ) > 0, INVALID_ARG );
   UTIL_ASSERT( ulBufferLen > 0, INVALID_ARG );
   RETURN_ON_NULL( s_pReader );
   memset( pszDataBuffer, 0, ulBufferLen );

   while( !bFound && ( xmlTextReaderRead( s_pReader ) == 1 ) )
   {
      const xmlChar *pszName = xmlTextReaderConstName( s_pReader );
      if( pszName != _null_ )
      {
         bFound = ( strcmp( pszName, pszElementName ) == 0 );
      }
   }

   if( bFound )
   {
      // We have the first tag
      xmlTextReaderRead( s_pReader );
      strncpy( pszDataBuffer, xmlTextReaderConstValue( s_pReader ), ulBufferLen - 1 );
      // this should close the tag
      xmlTextReaderRead( s_pReader );
   }

   return NO_ERROR;
}


ERROR_CODE ReadXml(const char* pszFilename, const UTIL_STR_ARRAY* psKeys, UTIL_STR_ARRAY* psStrArray)
{
    ERROR_CODE eRet = NO_ERROR;
    xmlTextReaderPtr pReader = NULL;
    uint32_t x = 0;

    RETURN_ON_NULL( pszFilename );
    RETURN_ON_NULL( psKeys );
    RETURN_ON_NULL( psStrArray );

    if (pszFilename == NULL || psKeys == NULL || psStrArray == NULL)
    {
        return INVALID_ARG;
    }

    memset(psStrArray, 0, sizeof(UTIL_STR_ARRAY));

    RETURN_ON_FAIL( OpenXmlFile( pszFilename ) );

    while( x < CONFIG_LAST && strlen( psKeys->aszStringArray[x] ) > 0 )
    {
       RETURN_ON_FAIL( ExtractDataFromElement( psKeys->aszStringArray[x], psStrArray->aszStringArray[x], sizeof( psStrArray->aszStringArray[x] ) ) );
       printf( "%s->%s: %s\n", pszFilename, psKeys->aszStringArray[x], psStrArray->aszStringArray[x] );
       x++;
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

ERROR_CODE WriteXml( const char * pszFilename, const UTIL_STR_ARRAY * psConfigKeys, const UTIL_STR_ARRAY * psConfigValues )
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL;/* node pointers */
    char buff[256] = { 0, };
    int i = 0, j = 0;

    if( pszFilename == NULL || psConfigKeys == NULL || psConfigValues == NULL )
    {
        return INVALID_ARG;
    }

    LIBXML_TEST_VERSION;

    /*
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc( BAD_CAST "1.0" );
    root_node = xmlNewNode( NULL, BAD_CAST "root" );
    xmlDocSetRootElement( doc, root_node );

    /*
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node.
     */

    while( i < CONFIG_LAST && strlen( psConfigKeys->aszStringArray[i] ) > 0 && strlen( psConfigValues->aszStringArray[i] ) > 0 )
    {
        xmlNewChild( root_node, NULL, BAD_CAST psConfigKeys->aszStringArray[i], BAD_CAST psConfigValues->aszStringArray[i] );
        i++;
    }

    /*
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc( pszFilename, doc, "UTF-8", 1 );

    /*free the document */
    xmlFreeDoc( doc );

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return NO_ERROR;
}
