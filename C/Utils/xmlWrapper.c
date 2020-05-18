/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

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


ERROR_CODE FindElement( const xmlWrapperPtr psFilePtr, const char *pszElementName, bool *pbFound )
{
   xmlTextReaderPtr psReader = 0;
   bool bFound = false;

   RETURN_ON_NULL( psFilePtr );
   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( pbFound );
   *pbFound = false;
   psReader = ( xmlTextReaderPtr )psFilePtr;

   while( !bFound && ( xmlTextReaderRead( psReader ) == 1 ) )
   {
      const xmlChar *pszName = xmlTextReaderConstName( psReader );
      if( pszName != _null_ )
      {
         bFound = ( strcmp( pszName, pszElementName ) == 0 );
      }
   }

   *pbFound = bFound;

   return bFound ? NO_ERROR : NOT_FOUND;
}

ERROR_CODE ExtractDataFromElement( const xmlWrapperPtr psFilePtr, const char* pszElementName, char* pszDataBuffer, uint32_t ulBufferLen )
{
   bool bFound = false;
   xmlChar* pszValue = _null_;
   xmlTextReaderPtr psReader = 0;

   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( pszDataBuffer );
   UTIL_ASSERT( strlen( pszElementName ) > 0, INVALID_ARG );
   UTIL_ASSERT( ulBufferLen > 0, INVALID_ARG );
   RETURN_ON_NULL( psFilePtr );

   memset( pszDataBuffer, 0, ulBufferLen );
   psReader = ( xmlTextReaderPtr )psFilePtr;
   RETURN_ON_FAIL( FindElement( psFilePtr, pszElementName, &bFound ) );

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

ERROR_CODE CreateDocPtr( xmlWriterPtrs *psXmlFile )
{
   xmlDocPtr psDoc = _null_;       /* Document pointer */
   xmlNodePtr root_node = _null_;  /* node pointers */
 
   RETURN_ON_NULL( psXmlFile );

   LIBXML_TEST_VERSION;

   /*
    * Creates a new Document, a node and set it as a root node
    */
   psDoc = xmlNewDoc( BAD_CAST "1.0" );
   root_node = xmlNewNode( _null_, BAD_CAST "root" );
   xmlDocSetRootElement( psDoc, root_node );

   psXmlFile->ppsDocPtr = ( xmlDocWriterPtr* )psDoc;
   psXmlFile->ppsRootNodePtr = ( xmlRootNodePtr* )root_node;

   return NO_ERROR;
}

ERROR_CODE CreateXmlNode( const xmlWriterPtrs *psXmlFile, const char *pszElement, const char *pszData )
{
   xmlNodePtr root_node = _null_;

   RETURN_ON_NULL( psXmlFile );
   RETURN_ON_NULL( psXmlFile->ppsDocPtr );
   RETURN_ON_NULL( psXmlFile->ppsRootNodePtr );
   RETURN_ON_NULL( pszElement );
   RETURN_ON_NULL( pszData );

   root_node = ( xmlNodePtr )psXmlFile->ppsRootNodePtr;

   xmlNewChild( root_node, _null_, BAD_CAST pszElement, BAD_CAST pszData );

   return NO_ERROR;
}

ERROR_CODE WriteXmlFile( const xmlWriterPtrs *psXmlFile, const char *pszFilename )
{
   xmlDocPtr psDoc = _null_;

   RETURN_ON_NULL( psXmlFile );
   RETURN_ON_NULL( psXmlFile->ppsDocPtr );
   RETURN_ON_NULL( pszFilename );

   psDoc = ( xmlDocPtr )psXmlFile->ppsDocPtr;

   xmlSaveFormatFileEnc( pszFilename, psDoc, "UTF-8", 1 );
   CleanupDumpXmlMemory();

   return NO_ERROR;
}
