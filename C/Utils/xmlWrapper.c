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
typedef struct
{
   char szOne[32 + 1];
   char szTwo[16 + 1];
} TEXT_STRUCT;

ERROR_CODE xmlWrapperParseFile( const char *pszFileName, XML_ITEM *pasItems, uint32_t ulArraySize, TEXT_STRUCT *psOutputStruct )
{
   RETURN_ON_NULL( pszFileName );
   RETURN_ON_NULL( pasItems );
   RETURN_ON_NULL( psOutputStruct );
   UTIL_ASSERT( ulArraySize != 0, INVALID_ARG );


// doc = xmlParseFile("config.xml");
//    if (!doc)
//    {
//       DBG_PRINTF("Couldn't open file");
//    }

//    xmlKeepBlanksDefault(0);
//    xmlNodePtr cur = xmlDocGetRootElement(doc);

//    DBG_PRINTF("Root Element of the xml is [%s]", BAD_CAST(cur->name));
//    cur = cur->xmlChildrenNode;
//    while( cur != _null_ && xmlIsBlankNode( cur ))
//    {
//       cur=cur->next;
//    }
//    DBG_PRINTF("Child Element of root is [%s], isEmpty [%d]", BAD_CAST(cur->name), xmlIsBlankNode(cur));

return NO_ERROR;
}

ERROR_CODE XmlTest(void)
{
   uint32_t ulTestCount = 0;
#define PRINTF_TEST(string) ( DBG_PRINTF( "----- %s | Test Count: %u -----", string, ulTestCount++ ) ) 

   {
      XML_ITEM asTest[2] = { 0, };
      TEXT_STRUCT sTest = { 0, };

      PRINTF_TEST( "Sanity Tests" );
      RETURN_ON_FAIL( xmlWrapperParseFile( _null_, _null_, _null_, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( "test.xml", _null_, _null_, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( "test.xml", &asTest[2], 0, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
      RETURN_ON_FAIL( xmlWrapperParseFile( "test.xml", &asTest[2], 2, _null_ ) == INVALID_ARG ? NO_ERROR : FAILED );
   }

#undef PRINTF_TEST
   return NO_ERROR;
}
