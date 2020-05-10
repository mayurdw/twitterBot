/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

// TODO: 
/*
   
*/

ERROR_CODE OpenXmlFile( xmlWrapperPtr *ppsFilePtr, const char* pszFilename )
{
   xmlTextReaderPtr pReader = _null_;

   RETURN_ON_NULL( ppsFilePtr );
   RETURN_ON_NULL( pszFilename );
   UTIL_ASSERT( strlen( pszFilename ) > 0, INVALID_ARG );

   if( *ppsFilePtr != _null_ )
   {
      CleanupDumpXmlMemory( );
      *ppsFilePtr = _null_;
   }

   pReader = xmlReaderForFile( pszFilename, _null_, 0 );
   RETURN_ON_NULL( pReader );

   *ppsFilePtr = ( xmlWrapperPtr * )pReader;

   return NO_ERROR;
}

ERROR_CODE ExtractDataFromElement( const xmlWrapperPtr psFilePtr, const char* pszElementName, char* pszDataBuffer, uint32_t ulBufferLen )
{
   bool bFound = FALSE;
   xmlChar* pszValue = _null_;
   xmlTextReaderPtr psReader = 0;

   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( pszDataBuffer );
   UTIL_ASSERT( strlen( pszElementName ) > 0, INVALID_ARG );
   UTIL_ASSERT( ulBufferLen > 0, INVALID_ARG );
   RETURN_ON_NULL( psFilePtr );

   memset( pszDataBuffer, 0, ulBufferLen );
   psReader = ( xmlTextReaderPtr )psFilePtr;

   while( !bFound && ( xmlTextReaderRead( psReader ) == 1 ) )
   {
      const xmlChar *pszName = xmlTextReaderConstName( psReader );
      if( pszName != _null_ )
      {
         bFound = ( strcmp( pszName, pszElementName ) == 0 );
      }
   }

   if( bFound )
   {
      // We have the first tag
      xmlTextReaderRead( psReader );
      Strcpy_safe( pszDataBuffer, xmlTextReaderConstValue( psReader ), ulBufferLen );
      // this should close the tag
      xmlTextReaderRead( psReader );
   }

   return NO_ERROR;
}

ERROR_CODE CleanupDumpXmlMemory( void )
{
   // /*
       //  * Cleanup function for the XML library.
       //  */
   xmlCleanupParser( );
   // /*
   //  * this is to debug memory for regression tests
   //  */
   xmlMemoryDump( );

}

ERROR_CODE WriteXml( const char * pszFilename, const UTIL_STR_ARRAY * psConfigKeys, const UTIL_STR_ARRAY * psConfigValues )
{
    xmlDocPtr doc = _null_;       /* document pointer */
    xmlNodePtr root_node = _null_, node = _null_;/* node pointers */
    char buff[256] = { 0, };
    int i = 0, j = 0;

    RETURN_ON_NULL( pszFilename );
    RETURN_ON_NULL( psConfigKeys );
    RETURN_ON_NULL( psConfigValues );

    LIBXML_TEST_VERSION;

    /*
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc( BAD_CAST "1.0" );
    root_node = xmlNewNode( _null_, BAD_CAST "root" );
    xmlDocSetRootElement( doc, root_node );

    /*
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node.
     */

    while( i < CONFIG_LAST && strlen( psConfigKeys->aszStringArray[i] ) > 0 && strlen( psConfigValues->aszStringArray[i] ) > 0 )
    {
        xmlNewChild( root_node, _null_, BAD_CAST psConfigKeys->aszStringArray[i], BAD_CAST psConfigValues->aszStringArray[i] );
        i++;
    }

    /*
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc( pszFilename, doc, "UTF-8", 1 );
    CleanupDumpXmlMemory( );

    return NO_ERROR;
}
