/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

ERROR_CODE OpenXmlFile(xmlWrapperPtr *ppsFilePtr, const char *pszFilename)
{
   xmlTextReaderPtr pReader = _null_;

   RETURN_ON_NULL(ppsFilePtr);
   RETURN_ON_NULL(pszFilename);
   UTIL_ASSERT(strlen(pszFilename) > 0, INVALID_ARG);

   if (*ppsFilePtr != _null_)
   {
      CleanupDumpXmlMemory();
      *ppsFilePtr = _null_;
   }

   pReader = xmlReaderForFile(pszFilename, _null_, 0);
   RETURN_ON_NULL(pReader);

   *ppsFilePtr = (xmlWrapperPtr *)pReader;

   return NO_ERROR;
}

ERROR_CODE FindElement(const xmlWrapperPtr psFilePtr, const char *pszElementName, bool *pbFound)
{
   xmlTextReaderPtr psReader = 0;
   bool bFound = false;

   RETURN_ON_NULL(psFilePtr);
   RETURN_ON_NULL(pszElementName);
   RETURN_ON_NULL(pbFound);
   *pbFound = false;
   psReader = (xmlTextReaderPtr)psFilePtr;

   while (!bFound && (xmlTextReaderRead(psReader) == 1))
   {
      const xmlChar *pszName = xmlTextReaderConstName(psReader);
      if (pszName != _null_)
      {
         bFound = (strcmp(pszName, pszElementName) == 0);
      }
   }

   *pbFound = bFound;

   return bFound ? NO_ERROR : NOT_FOUND;
}

ERROR_CODE ExtractDataFromElement(const xmlWrapperPtr psFilePtr, const char *pszElementName, char *pszDataBuffer, uint32_t ulBufferLen)
{
   bool bFound = false;
   xmlChar *pszValue = _null_;
   xmlTextReaderPtr psReader = 0;

   RETURN_ON_NULL(pszElementName);
   RETURN_ON_NULL(pszDataBuffer);
   UTIL_ASSERT(strlen(pszElementName) > 0, INVALID_ARG);
   UTIL_ASSERT(ulBufferLen > 0, INVALID_ARG);
   RETURN_ON_NULL(psFilePtr);

   memset(pszDataBuffer, 0, ulBufferLen);
   psReader = (xmlTextReaderPtr)psFilePtr;
   RETURN_ON_FAIL(FindElement(psFilePtr, pszElementName, &bFound));

   if (bFound)
   {
      // We have the first tag
      xmlTextReaderRead(psReader);
      Strcpy_safe(pszDataBuffer, xmlTextReaderConstValue(psReader), ulBufferLen);
      // this should close the tag
      xmlTextReaderRead(psReader);
   }

   return NO_ERROR;
}

ERROR_CODE CleanupDumpXmlMemory(void)
{
   // /*
   //  * Cleanup function for the XML library.
   //  */
   xmlCleanupParser();
   // /*
   //  * this is to debug memory for regression tests
   //  */
   xmlMemoryDump();
}

ERROR_CODE CreateDocPtr(xmlWriterPtrs *psXmlFile)
{
   xmlDocPtr psDoc = _null_;      /* Document pointer */
   xmlNodePtr root_node = _null_; /* node pointers */

   RETURN_ON_NULL(psXmlFile);

   LIBXML_TEST_VERSION;

   /*
    * Creates a new Document, a node and set it as a root node
    */
   psDoc = xmlNewDoc(BAD_CAST "1.0");
   root_node = xmlNewNode(_null_, BAD_CAST "root");
   xmlDocSetRootElement(psDoc, root_node);

   psXmlFile->ppsDocPtr = (xmlDocWriterPtr *)psDoc;
   psXmlFile->ppsRootNodePtr = (xmlRootNodePtr *)root_node;

   return NO_ERROR;
}

ERROR_CODE CreateXmlNode(const xmlWriterPtrs *psXmlFile, const char *pszElement, const char *pszData)
{
   xmlNodePtr root_node = _null_;

   RETURN_ON_NULL(psXmlFile);
   RETURN_ON_NULL(psXmlFile->ppsDocPtr);
   RETURN_ON_NULL(psXmlFile->ppsRootNodePtr);
   RETURN_ON_NULL(pszElement);
   RETURN_ON_NULL(pszData);

   root_node = (xmlNodePtr)psXmlFile->ppsRootNodePtr;

   xmlNewChild(root_node, _null_, BAD_CAST pszElement, BAD_CAST pszData);

   return NO_ERROR;
}

ERROR_CODE WriteXmlFile(const xmlWriterPtrs *psXmlFile, const char *pszFilename)
{
   xmlDocPtr psDoc = _null_;

   RETURN_ON_NULL(psXmlFile);
   RETURN_ON_NULL(psXmlFile->ppsDocPtr);
   RETURN_ON_NULL(pszFilename);

   psDoc = (xmlDocPtr)psXmlFile->ppsDocPtr;

   xmlSaveFormatFileEnc(pszFilename, psDoc, "UTF-8", 1);
   CleanupDumpXmlMemory();

   return NO_ERROR;
}

typedef enum
{
   XML_CHILD_STRING = 0,
   XML_SUB_TABLE,
   XML_ARRAY
} XML_TYPES;

typedef struct
{
   char *pszElementName;
   uint32_t ulMemberOffset;
   uint32_t ulBufferSize;
   XML_TYPES eType;
} XML_ITEM;

#define XML_STR(element, structure, var) element, offsetof(structure, var), sizeof(((structure *)0)->var), XML_CHILD_STRING
// TODO: Add table elements?
#define XML_SUB_TABLE( element, structure, var) element, offsetof(structure,var), sizeof(((structure*)0)->var), XML_SUB_TABLE
// TODO: Add number of items
#define XML_ARRAY(element,structure,var) element, offsetof(structure,var), sizeof(((structure*)0)->var),XML_ARRAY

ERROR_CODE xmlWrapperParseFile( const char *pszFileName, const XML_ITEM *pasItems, uint32_t ulArraySize, void *pvOutputStruct )
{
   xmlDocPtr pDoc = _null_;
   xmlNodePtr pNode = _null_;

   RETURN_ON_NULL( pszFileName );
   RETURN_ON_NULL( pasItems );
   RETURN_ON_NULL( pvOutputStruct );
   UTIL_ASSERT( ulArraySize != 0, INVALID_ARG );

   // Not sure what it does exactly, but adding it anyway
   xmlKeepBlanksDefault(0);

   pDoc = xmlParseFile( pszFileName );
   if( !pDoc )
   {
      DBG_PRINTF( "File Couldn't be opened" );
      return FILE_ERROR;
   }

   // Ignore root node. But make sure it has one
   pNode = xmlDocGetRootElement( pDoc );
   if( pNode != _null_ )
   {
      uint32_t ulCount = 0;
      //DBG_PRINTF( "Root Element = [%s]", BAD_CAST( pNode->name ) );

      pNode = pNode->xmlChildrenNode;
      while( pNode != _null_ && ulCount <= ulArraySize )
      {
         while( pNode != _null_ && xmlIsBlankNode( pNode ) )
         {
            pNode=pNode->next;
         }
         //DBG_PRINTF( "Child Node is [%s]", BAD_CAST( pNode->name ) );
         if( strcmp( pasItems[ulCount].pszElementName, BAD_CAST( pNode->name ) ) == 0 )
         {
            xmlChar *pKey = xmlNodeListGetString( pDoc, pNode->xmlChildrenNode, 1 );

            Strcpy_safe( ( char* )( pvOutputStruct + pasItems[ulCount].ulMemberOffset ), ( const char* )pKey, pasItems[ulCount].ulBufferSize );

            xmlFree( pKey );
            ulCount++;
         }

         pNode=pNode->next;
      }
   }

   xmlFreeDoc( pDoc );
   return NO_ERROR;
}

ERROR_CODE XmlTest(void)
{
   uint32_t ulTestCount = 0;
   const char *pszFileName = "text.xml";

#define PRINTF_TEST(string) ( DBG_PRINTF( "----- %s | Test Count: %u -----", string, ulTestCount++ ) ) 

   {
      typedef struct{} EMPTY_STRUCT;
      XML_ITEM asTest[2] = { 0, };
      EMPTY_STRUCT sTest;

      PRINTF_TEST( "Sanity Tests" );
      RETURN_ON_FAIL( xmlWrapperParseFile( _null_, _null_, _null_, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, _null_, _null_, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, &asTest[2], 0, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, &asTest[2], 2, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( "temp.xml", &asTest[2], 2, &sTest ) == FILE_ERROR ? NO_ERROR : FAILED );
   }

   {
      typedef struct
      {
         char szTo[8+1];
         char szFrom[8+1];
         char szHeading[16+1];
         char szBody[64+1];
      } BASIC_FILE;
      BASIC_FILE sBasicFile = { 0, };
      const XML_ITEM asItems[] =
      {
         XML_STR( "to", BASIC_FILE, szTo ),
         XML_STR( "from", BASIC_FILE, szFrom ),
         XML_STR( "heading", BASIC_FILE, szHeading ),
         XML_STR( "body", BASIC_FILE, szBody )
      };
#define TO        "Tove"
#define FROM      "Jani"
#define HEADING   "Reminder"
#define BODY      "Don't forget me this weekend!"
      const char *pszFileData = 
         "<note>"
         "<to>" TO "</to>"
         "<from>" FROM "</from>"
         "<heading>" HEADING "</heading>"
         "<body>" BODY "</body>"
         "</note>";
      FILE *pFile = _null_;
      uint32_t ulBytesWritten = 0;

      PRINTF_TEST( "Basic single layer file" );
      pFile = fopen( pszFileName, "w" );
      if( pFile == _null_ )
      {
         DBG_PRINTF( "Couldn't write to file" );
      }
      ulBytesWritten = fwrite( pszFileData, 1, strlen( pszFileData ), pFile );
      fclose( pFile );
      if( ulBytesWritten != strlen( pszFileData ) )
      {
         DBG_PRINTF( "Couldn't write [%d] number of bytes, only wrote [%u] bytes", strlen( pszFileData ), ulBytesWritten );
      }

      RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, asItems, ( sizeof( asItems ) / sizeof( asItems[0] ) ), &sBasicFile ) );
#if 0
      DBG_PRINTF( "Items   = " );
      DBG_PRINTF( "To      = [%s]", sBasicFile.szTo );
      DBG_PRINTF( "From    = [%s]", sBasicFile.szFrom );
      DBG_PRINTF( "Heading = [%s]", sBasicFile.szHeading );
      DBG_PRINTF( "Body    = [%s]", sBasicFile.szBody );
#endif
      RETURN_ON_FAIL( strcmp( sBasicFile.szTo, TO ) == 0 ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( strcmp( sBasicFile.szFrom, FROM ) == 0 ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( strcmp( sBasicFile.szHeading, HEADING ) == 0 ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( strcmp( sBasicFile.szBody, BODY ) == 0 ? NO_ERROR : FAILED );
   }

#undef PRINTF_TEST
   DBG_PRINTF( "All [%u] tests successfully passed", ulTestCount );
   
   return NO_ERROR;
}
