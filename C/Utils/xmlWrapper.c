/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <libxml/xpath.h>
#include <libxml/xmlstring.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "xmlWrapper.h"

// Defines
#define XML_DEBUG ( 0 )


// Static Functions
static ERROR_CODE xmlWrapperExtractChildString( const xmlDocPtr pDoc, const XML_ITEM *psItem, void *pvOutputStruct, const xmlChar *pszPrefix, const xmlXPathContextPtr pContext );
static xmlXPathObjectPtr xmlWrapperExtractNodeSetPtr( const xmlDocPtr pDoc, const char *pszPrefix, const char *pszElementName, const xmlXPathContextPtr pContext );

////////////////////////////////////////////////////////////////

static xmlXPathObjectPtr xmlWrapperExtractNodeSetPtr( const xmlDocPtr pDoc, const char *pszPrefix, const char *pszElementName, const xmlXPathContextPtr pContext )
{
   xmlChar szKey[32+1] = { 0, };

   if( !pDoc || !pszElementName || !pszPrefix || !pContext )
      return _null_;
      
   xmlStrPrintf( szKey, sizeof( szKey ), "%s/%s", pszPrefix, pszElementName );
   
   return xmlXPathEvalExpression( szKey, pContext );
}

static ERROR_CODE xmlWrapperExtractChildString( const xmlDocPtr pDoc, const XML_ITEM *psItem, void *pvOutputStruct, const xmlChar *pszPrefix, const xmlXPathContextPtr pContext )
{
   RETURN_ON_NULL( pDoc );
   RETURN_ON_NULL( psItem );
   RETURN_ON_NULL( pvOutputStruct );
   RETURN_ON_NULL( pszPrefix );
   RETURN_ON_NULL( pContext );

   xmlXPathObjectPtr pXpathObject = xmlWrapperExtractNodeSetPtr( pDoc, pszPrefix, psItem->pszElementName, pContext );
   if( _null_ == pXpathObject || xmlXPathNodeSetIsEmpty( pXpathObject->nodesetval ) )
   {
#if XML_DEBUG
      DBG_PRINTF( "Not found" );
#endif
      memset( ( pvOutputStruct + psItem->ulMemberOffset ), 0, psItem->ulBufferSize );
   }
   else 
   {
      xmlNodeSetPtr nodeset = pXpathObject->nodesetval;
      for ( int i=0; i < nodeset->nodeNr; i++) 
      {
         xmlChar *pData = xmlNodeListGetString( pDoc, nodeset->nodeTab[i]->xmlChildrenNode, 1 );
#if XML_DEBUG
         DBG_PRINTF( "String found: [%s]", pData );
#endif
         Strcpy_safe( ( pvOutputStruct + psItem->ulMemberOffset ), pData, psItem->ulBufferSize );
         xmlFree( pData );
      } 
   }
   xmlXPathFreeObject( pXpathObject );

   return NO_ERROR;
}

ERROR_CODE xmlWrapperParseFile( const char *pszFileName, const XML_ITEM *pasItems, uint32_t ulArraySize, void *pvOutputStruct )
{
   xmlDocPtr pDoc = _null_;
   xmlNodePtr pNode = _null_;
   xmlXPathContextPtr pContext = _null_ ;

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

   pContext = xmlXPathNewContext( pDoc );
   if( _null_ == pContext )
   {
      DBG_PRINTF( "XPathContext couldn't be inialized" );
   }
   else
   {
      /* code */
      uint32_t ulCount = 0;
      
      while( ulCount < ulArraySize )
      {
         switch( pasItems[ulCount].eType )
         {
            case XML_CHILD_STRING: 
               RETURN_ON_FAIL( xmlWrapperExtractChildString( pDoc, &pasItems[ulCount], pvOutputStruct, "/", pContext ) );
               break;

            case XML_TABLE:
            {
               XML_ITEM *pasTable = ( XML_ITEM * )pasItems[ulCount].pavSubItem;
               uint32_t ulIndex = 0, ulOffset = sizeof( XML_ITEM );
               xmlChar szPrefix[32+1] = { 0, };

               RETURN_ON_NULL( pasTable );
               UTIL_ASSERT( pasItems[ulCount].ulArrayElements != 0, INVALID_ARG );

               while( ulIndex < pasItems[ulCount].ulArrayElements )
               {
                  memset( szPrefix, 0, sizeof( szPrefix ) );
                  xmlStrPrintf( szPrefix, sizeof( szPrefix ), "//%s", pasItems[ulCount].pszElementName );
                  RETURN_ON_FAIL( xmlWrapperExtractChildString( pDoc, &pasTable[ulIndex], ( pvOutputStruct + pasItems[ulCount].ulMemberOffset ), szPrefix, pContext ) ); 
                  ulIndex++;
               }
            }
            break;

            case XML_SUB_ARRAY: 
            {
               // Not going to verify if all of the elements are from the same parent
               XML_ITEM *pasTable = ( XML_ITEM * )pasItems[ulCount].pavSubItem;
               uint32_t ulIndex = 0;
               xmlChar szPrefix[32+1] = { 0, };
               uint32_t ulSingleIndexSize = pasItems[ulCount].ulBufferSize / pasItems[ulCount].ulArraySize;

               RETURN_ON_NULL( pasTable );
               UTIL_ASSERT( pasItems[ulCount].ulArrayElements != 0, INVALID_ARG );

               while( ulIndex < pasItems[ulCount].ulArrayElements )
               {
                  uint32_t ulInternalIndex = 0;

                  memset( szPrefix, 0, sizeof( szPrefix ) );

                  xmlStrPrintf( szPrefix, sizeof( szPrefix ), "//%s", pasItems[ulCount].pszElementName );
                  // Get xmlXPathObjectPtr
                  xmlXPathObjectPtr pXpathObject = xmlWrapperExtractNodeSetPtr( pDoc, szPrefix, pasTable[ulIndex].pszElementName, pContext );

                  if( _null_ == pXpathObject || xmlXPathNodeSetIsEmpty( pXpathObject->nodesetval ) )
                  {
#if XML_DEBUG
                     DBG_PRINTF( "Not found" );
#endif
                  }
                  else
                  {
                     xmlNodeSetPtr nodeset = pXpathObject->nodesetval;

                     for ( int i=0; i < nodeset->nodeNr && i < pasItems[ulCount].ulArraySize; i++) 
                     {
                        xmlChar *pData = xmlNodeListGetString( pDoc, nodeset->nodeTab[i]->xmlChildrenNode, 1 );
                        uint32_t ulOffset = pasItems[ulCount].ulMemberOffset + pasTable[ulIndex].ulMemberOffset + ( ulSingleIndexSize * i );
#if XML_DEBUG
                        DBG_PRINTF( "String found: [%s]", pData );
#endif
                        Strcpy_safe( 
                        ( pvOutputStruct + ulOffset ), 
                        pData, 
                        pasTable[ulIndex].ulBufferSize );
                        xmlFree( pData );
                     }
                  }
                  
                  ulIndex++;
               }
            }
            break;

            default: DBG_PRINTF( "Unknown type or hasn't been implemented yet = [%d]", pasItems[ulCount].eType ); break;
         }
         ulCount++;
      }
   }
   
   xmlFreeDoc( pDoc );
   xmlCleanupParser();

   return NO_ERROR;
}

#define MY_ENCODING     "UTF-8"
ERROR_CODE xmlWrapperWriteFile( const char *pszFileName, const XML_ITEM *pasItems, uint32_t ulArraySize, const void *pvInputStruct )
{
   int iRet = 0;
   xmlTextWriterPtr pWriter = _null_;
   uint32_t ulCount = 0;

   RETURN_ON_NULL( pszFileName );
   RETURN_ON_NULL( pasItems );
   RETURN_ON_NULL( pvInputStruct );
   UTIL_ASSERT( ulArraySize != 0, INVALID_ARG );


   /* Create a new XmlWriter for uri, with no compression. */
   pWriter = xmlNewTextWriterFilename(pszFileName, 0);
   if (pWriter == NULL) {
      DBG_PRINTF("Error creating the xml pWriter = [%d]", iRet );
      return FAILED;
   }

   iRet = xmlTextWriterStartDocument(pWriter, NULL, MY_ENCODING, NULL);
   if (iRet < 0) {
      DBG_PRINTF( "testXmlwriterFilename: Error at xmlTextWriterStartDocument" );
      return FAILED;
   }

   // Parent element
   iRet = xmlTextWriterStartElement(
      pWriter, 
      BAD_CAST "root" );
   if (iRet < 0) {
      DBG_PRINTF("testXmlwriterFilename: Error at xmlTextWriterStartElement");
      return FAILED;
   }


   while( ulCount < ulArraySize )
   {
      switch (pasItems[ulCount].eType)
      {
      case XML_CHILD_STRING:
         /* Write an element named "CUSTOMER_ID" as child of HEADER. */
         iRet = xmlTextWriterWriteFormatElement(
            pWriter, 
            BAD_CAST pasItems[ulCount].pszElementName,
            "%s", 
            ( const char * )( pvInputStruct + pasItems[ulCount].ulMemberOffset ) );
         if (iRet < 0) 
         {
            DBG_PRINTF( "testXmlwriterFilename: Error at xmlTextWriterWriteFormatElement" );
            return FAILED;
         }
         break;
      
#if 0

    iRet = xmlTextWriterStartElement(pWriter, BAD_CAST "HEADER");
    if (iRet < 0) {
        DBG_PRINTF
            ("testXmlwriterFilename: Error at xmlTextWriterStartElement");
        return FAILED;
    }

    /* Close the element named HEADER. */
    iRet = xmlTextWriterEndElement(pWriter);
    if (iRet < 0) {
        DBG_PRINTF
            ("testXmlwriterFilename: Error at xmlTextWriterEndElement");
        return FAILED;
    }

#endif

      default:
         DBG_PRINTF( "Unknown element [%d], ignoring the element" );
         break;
      }
      ulCount++;
   }

    iRet = xmlTextWriterEndDocument(pWriter);
    if (iRet < 0) {
        DBG_PRINTF
            ("testXmlwriterFilename: Error at xmlTextWriterEndDocument");
        return FAILED;
    }

    xmlFreeTextWriter(pWriter);

   return NO_ERROR;
}

//////////////////////////////////////////////////////////////////

static uint32_t s_ulTestCount = 0;
#if XML_DEBUG
#define PRINTF_TEST(string) ( DBG_PRINTF( "----- %s | Test Count: %u -----", string, s_ulTestCount++ ) ) 
#else
#define PRINTF_TEST(string) ( s_ulTestCount++ )
#endif

static ERROR_CODE xmlTestSanityTest( const char *pszFileName )
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

   return NO_ERROR;
}

static ERROR_CODE xmlTestBasicSingleLayer( const char *pszFileName )
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
#if XML_DEBUG
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

#undef TO       
#undef FROM     
#undef HEADING  
#undef BODY     

   return NO_ERROR;
}

static ERROR_CODE xmlTestOutOfOrderSingleLayer( const char *pszFileName )
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
      "<from>" FROM "</from>"
      "<to>" TO "</to>"
      "<body>" BODY "</body>"
      "<heading>" HEADING "</heading>"
      "</note>";
   FILE *pFile = _null_;
   uint32_t ulBytesWritten = 0;

   PRINTF_TEST( "Same sample file but out of order" );
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
#if XML_DEBUG
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

#undef TO        
#undef FROM      
#undef HEADING   
#undef BODY      

   return NO_ERROR;
}

static ERROR_CODE xmlTestSimpleSubTable( const char *pszFileName )
{
   typedef struct
   {
      char szTo[8+1];
      char szFrom[8+1];
      char szHeading[16+1];
      char szBody[64+1];
   } BASIC_FILE;
   typedef struct 
   {
      BASIC_FILE sFile;
   } WRAPPER_FILE;
   WRAPPER_FILE sBasicFile = { 0, };
   const XML_ITEM asNote[] =
   {
      XML_STR( "to", BASIC_FILE, szTo ),
      XML_STR( "from", BASIC_FILE, szFrom ),
      XML_STR( "heading", BASIC_FILE, szHeading ),
      XML_STR( "body", BASIC_FILE, szBody )
   };
   const XML_ITEM asItems[] = 
   {
      XML_SUB_TABLE( "note", WRAPPER_FILE, sFile, asNote, ARRAY_COUNT( asNote ) )
   };
#define TO        "Tove"
#define FROM      "Jani"
#define HEADING   "Reminder"
#define BODY      "Don't forget me this weekend!"
   const char *pszFileData = 
      "<root>"
         "<note>"
            "<to>" TO "</to>"
            "<from>" FROM "</from>"
            "<heading>" HEADING "</heading>"
            "<body>" BODY "</body>"
         "</note>"
      "</root>";
   FILE *pFile = _null_;
   uint32_t ulBytesWritten = 0;

   PRINTF_TEST( "Embedded subtable" );
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
#if XML_DEBUG
   DBG_PRINTF( "Items   = " );
   DBG_PRINTF( "To      = [%s]", sBasicFile.sFile.szTo );
   DBG_PRINTF( "From    = [%s]", sBasicFile.sFile.szFrom );
   DBG_PRINTF( "Heading = [%s]", sBasicFile.sFile.szHeading );
   DBG_PRINTF( "Body    = [%s]", sBasicFile.sFile.szBody );
#endif
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szTo, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szFrom, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szHeading, HEADING ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szBody, BODY ) == 0 ? NO_ERROR : FAILED );

#undef TO        
#undef FROM      
#undef HEADING   
#undef BODY      

   return NO_ERROR;
}

static ERROR_CODE xmlTestSubTableWithSiblingChild( const char *pszFileName )
{
   typedef struct
   {
      char szTo[8+1];
      char szFrom[8+1];
      char szHeading[16+1];
      char szBody[64+1];
   } BASIC_FILE;
   typedef struct 
   {
      char szDetails[16+1];
      BASIC_FILE sFile;
   } WRAPPER_FILE;
   WRAPPER_FILE sBasicFile = { 0, };
   const XML_ITEM asNote[] =
   {
      XML_STR( "to", BASIC_FILE, szTo ),
      XML_STR( "from", BASIC_FILE, szFrom ),
      XML_STR( "heading", BASIC_FILE, szHeading ),
      XML_STR( "body", BASIC_FILE, szBody )
   };
   const XML_ITEM asItems[] = 
   {
      XML_STR( "details", WRAPPER_FILE, szDetails ),
      XML_SUB_TABLE( "note", WRAPPER_FILE, sFile, asNote, ARRAY_COUNT( asNote ) )
   };
#define DETAILS   "Details"
#define TO        "Tove"
#define FROM      "Jani"
#define HEADING   "Reminder"
#define BODY      "Don't forget me this weekend!"
   const char *pszFileData = 
      "<root>"
         "<details>" DETAILS "</details>"
         "<note>"
            "<to>" TO "</to>"
            "<from>" FROM "</from>"
            "<heading>" HEADING "</heading>"
            "<body>" BODY "</body>"
         "</note>"
      "</root>";
   FILE *pFile = _null_;
   uint32_t ulBytesWritten = 0;

   PRINTF_TEST( "Subtable & extra child" );
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
#if XML_DEBUG
   DBG_PRINTF( "Items   = " );
   DBG_PRINTF( "Details = [%s]", sBasicFile.szDetails );
   DBG_PRINTF( "To      = [%s]", sBasicFile.sFile.szTo );
   DBG_PRINTF( "From    = [%s]", sBasicFile.sFile.szFrom );
   DBG_PRINTF( "Heading = [%s]", sBasicFile.sFile.szHeading );
   DBG_PRINTF( "Body    = [%s]", sBasicFile.sFile.szBody );
#endif
   RETURN_ON_FAIL( strcmp( sBasicFile.szDetails, DETAILS ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szTo, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szFrom, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szHeading, HEADING ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sFile.szBody, BODY ) == 0 ? NO_ERROR : FAILED );

#undef DETAILS
#undef TO        
#undef FROM      
#undef HEADING   
#undef BODY      

   return NO_ERROR;
}

static ERROR_CODE xmlTestSimpleArray( const char *pszFileName )
{
    typedef struct
   {
      char szTo[8+1];
      char szFrom[8+1];
      char szHeading[16+1];
      char szBody[64+1];
   } BASIC_FILE;
   const XML_ITEM asItem[] =
   {
      XML_STR( "to", BASIC_FILE, szTo ),
      XML_STR( "from", BASIC_FILE, szFrom ),
      XML_STR( "heading", BASIC_FILE, szHeading ),
      XML_STR( "body", BASIC_FILE, szBody )
   };

   typedef struct
   {
      BASIC_FILE asFile[10];
   } WRAPPER_FILE;
   WRAPPER_FILE sBasicFile = { 0, };

   const XML_ITEM asItems[] = 
   {
      XML_ARRAY( "note", WRAPPER_FILE, asFile, asItem, ARRAY_COUNT( asItem ), ARRAY_COUNT( sBasicFile.asFile ) )
   };

#define TO        "Tove"
#define FROM      "Jani"
#define HEADING   "Reminder"
#define REHEADING "Re: Reminder"
#define BODY      "Don't forget me this weekend!"
#define RESPONSE  "I will not!"
   const char *pszFileData = 
      "<root>"
         "<note>"
            "<to>" TO "</to>"
            "<from>" FROM "</from>"
            "<heading>" HEADING "</heading>"
            "<body>" BODY "</body>"
         "</note>"
         "<note>"
            "<to>" FROM "</to>"
            "<from>" TO "</from>"
            "<heading>" REHEADING "</heading>"
            "<body>" RESPONSE "</body>"
         "</note>"
      "</root>";
   FILE *pFile = _null_;
   uint32_t ulBytesWritten = 0;

   PRINTF_TEST( "Simple Array test" );

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
#if XML_DEBUG
   DBG_PRINTF( "Items   = " );
   DBG_PRINTF( "To      = [%s]", sBasicFile.asFile[0].szTo );
   DBG_PRINTF( "From    = [%s]", sBasicFile.asFile[0].szFrom );
   DBG_PRINTF( "Heading = [%s]", sBasicFile.asFile[0].szHeading );
   DBG_PRINTF( "Body    = [%s]", sBasicFile.asFile[0].szBody );
   DBG_PRINTF( "To      = [%s]", sBasicFile.asFile[1].szTo );
   DBG_PRINTF( "From    = [%s]", sBasicFile.asFile[1].szFrom );
   DBG_PRINTF( "Heading = [%s]", sBasicFile.asFile[1].szHeading );
   DBG_PRINTF( "Body    = [%s]", sBasicFile.asFile[1].szBody );
#endif

   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szTo, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szFrom, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szHeading, HEADING ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szBody, BODY ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szTo, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szFrom, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szHeading, REHEADING ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szBody, RESPONSE ) == 0 ? NO_ERROR : FAILED );
   

#undef TO        
#undef FROM      
#undef HEADING   
#undef BODY      
#undef REHEADING
#undef RESPONSE

   return NO_ERROR;
}

static ERROR_CODE xmlTestArrayWithSubTableAndSibling( const char *pszFileName )
{
   typedef struct
   {
      char szTo[8+1];
      char szFrom[8+1];
      char szHeading[16+1];
      char szBody[64+1];
   } BASIC_FILE;
   const XML_ITEM asItem[] =
   {
      XML_STR( "to", BASIC_FILE, szTo ),
      XML_STR( "from", BASIC_FILE, szFrom ),
      XML_STR( "heading", BASIC_FILE, szHeading ),
      XML_STR( "body", BASIC_FILE, szBody )
   };
   typedef struct TEST_STRUCT
   {
      char szOne[4+1];
      char szTwo[4+1];
   } TEST_STRUCT;
   
   const XML_ITEM asTests[] = 
   {
      XML_STR( "one", TEST_STRUCT, szOne ),
      XML_STR( "two", TEST_STRUCT, szTwo )
   };

   typedef struct
   {
      char szDetails[16+1];
      TEST_STRUCT sTest;
      BASIC_FILE asFile[10];
   } WRAPPER_FILE;
   WRAPPER_FILE sBasicFile = { 0, };

   const XML_ITEM asItems[] = 
   {
      XML_STR( "details", WRAPPER_FILE, szDetails ),
      XML_SUB_TABLE( "test", WRAPPER_FILE, sTest, asTests, ARRAY_COUNT( asTests ) ),
      XML_ARRAY( "note", WRAPPER_FILE, asFile, asItem, ARRAY_COUNT( asItem ), ARRAY_COUNT( sBasicFile.asFile ) )
   };

#define DETAILS   "Details of notes"
#define ONE       "one"
#define TWO       "two"
#define TO        "Tove"
#define FROM      "Jani"
#define HEADING   "Reminder"
#define REHEADING "Re: Reminder"
#define BODY      "Don't forget me this weekend!"
#define RESPONSE  "I will not!"
   const char *pszFileData = 
      "<root>"
         "<details>" DETAILS "</details>"
         "<test>"
            "<one>" ONE "</one>"
            "<two>" TWO "</two>"
         "</test>"
         "<note>"
            "<to>" TO "</to>"
            "<from>" FROM "</from>"
            "<heading>" HEADING "</heading>"
            "<body>" BODY "</body>"
         "</note>"
         "<note>"
            "<to>" FROM "</to>"
            "<from>" TO "</from>"
            "<heading>" REHEADING "</heading>"
            "<body>" RESPONSE "</body>"
         "</note>"
      "</root>";
   FILE *pFile = _null_;
   uint32_t ulBytesWritten = 0;

   PRINTF_TEST( "Array test with subtable & child" );

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

   RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, asItems, ARRAY_COUNT( asItems ), &sBasicFile ) );
#if XML_DEBUG
   DBG_PRINTF( "Items   = " );
   DBG_PRINTF( "Details = [%s]", sBasicFile.szDetails );
   DBG_PRINTF( "One     = [%s]", sBasicFile.sTest.szOne );
   DBG_PRINTF( "Two     = [%s]", sBasicFile.sTest.szTwo );
   DBG_PRINTF( "To      = [%s]", sBasicFile.asFile[0].szTo );
   DBG_PRINTF( "From    = [%s]", sBasicFile.asFile[0].szFrom );
   DBG_PRINTF( "Heading = [%s]", sBasicFile.asFile[0].szHeading );
   DBG_PRINTF( "Body    = [%s]", sBasicFile.asFile[0].szBody );
   DBG_PRINTF( "To      = [%s]", sBasicFile.asFile[1].szTo );
   DBG_PRINTF( "From    = [%s]", sBasicFile.asFile[1].szFrom );
   DBG_PRINTF( "Heading = [%s]", sBasicFile.asFile[1].szHeading );
   DBG_PRINTF( "Body    = [%s]", sBasicFile.asFile[1].szBody );
#endif

   RETURN_ON_FAIL( strcmp( sBasicFile.szDetails, DETAILS ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sTest.szOne, ONE ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.sTest.szTwo, TWO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szTo, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szFrom, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szHeading, HEADING ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[0].szBody, BODY ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szTo, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szFrom, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szHeading, REHEADING ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sBasicFile.asFile[1].szBody, RESPONSE ) == 0 ? NO_ERROR : FAILED );
   
#undef TO        
#undef FROM      
#undef HEADING   
#undef BODY      
#undef REHEADING
#undef RESPONSE
#undef ONE
#undef TWO
#undef DETAILS

   return NO_ERROR;
}

static ERROR_CODE xmlTestWriteSimpleLayer( const char *pszFileName )
{
   typedef struct 
   {
      char szTo[8+1];
      char szFrom[8+1];
      char szSubject[16+1];
      char szBody[64+1];
   } SIMPLE_LAYER;
   SIMPLE_LAYER sWriteLayer = {0,}, sReadLayer = {0,};
   const XML_ITEM asItems[] =
   {
      XML_STR( "to", SIMPLE_LAYER, szTo ),
      XML_STR( "from", SIMPLE_LAYER, szFrom ),
      XML_STR( "subject", SIMPLE_LAYER, szSubject ),
      XML_STR( "body", SIMPLE_LAYER, szBody )
   };
#define TO        "Tove"
#define FROM      "Jani"
#define SUBJECT   "Reminder"
#define BODY      "Don't forget about me"
   // Copy elements

   PRINTF_TEST( "Writing simple single layer" );
   Strcpy_safe( sWriteLayer.szTo, TO, sizeof( sWriteLayer.szTo ) );
   Strcpy_safe( sWriteLayer.szFrom, FROM, sizeof( sWriteLayer.szFrom ) );
   Strcpy_safe( sWriteLayer.szSubject, SUBJECT, sizeof( sWriteLayer.szSubject ) );
   Strcpy_safe( sWriteLayer.szBody, BODY, sizeof( sWriteLayer.szBody ) );

   // Write into file
   RETURN_ON_FAIL( xmlWrapperWriteFile( pszFileName, asItems, ARRAY_COUNT(asItems), &sWriteLayer ) );
#if XML_DEBUG
   DBG_PRINTF( "Written Items = " );
   DBG_PRINTF( "To =       [%s]", sWriteLayer.szTo );
   DBG_PRINTF( "From =     [%s]", sWriteLayer.szFrom );
   DBG_PRINTF( "Subject =  [%s]", sWriteLayer.szSubject );
   DBG_PRINTF( "Body =     [%s]", sWriteLayer.szBody );
#endif 

   RETURN_ON_FAIL( xmlWrapperParseFile( pszFileName, asItems, ARRAY_COUNT(asItems), &sReadLayer ) );

#if XML_DEBUG
   DBG_PRINTF( "Items = " );
   DBG_PRINTF( "To =       [%s]", sReadLayer.szTo );
   DBG_PRINTF( "From =     [%s]", sReadLayer.szFrom );
   DBG_PRINTF( "Subject =  [%s]", sReadLayer.szSubject );
   DBG_PRINTF( "Body =     [%s]", sReadLayer.szBody );
#endif 

   RETURN_ON_FAIL( strcmp( sReadLayer.szTo, TO ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sReadLayer.szFrom, FROM ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sReadLayer.szSubject, SUBJECT ) == 0 ? NO_ERROR : FAILED );
   RETURN_ON_FAIL( strcmp( sReadLayer.szBody, BODY ) == 0 ? NO_ERROR : FAILED );

   // Success
#undef TO
#undef FROM
#undef SUBJECT
#undef BODY
   return NO_ERROR;
}

ERROR_CODE XmlTest(void)
{
   const char *pszFileName = "text.xml";

   RETURN_ON_FAIL( xmlTestSanityTest( pszFileName ) );
   RETURN_ON_FAIL( xmlTestBasicSingleLayer( pszFileName ) );
   RETURN_ON_FAIL( xmlTestOutOfOrderSingleLayer( pszFileName ) );
   RETURN_ON_FAIL( xmlTestSimpleSubTable( pszFileName ) );
   RETURN_ON_FAIL( xmlTestSubTableWithSiblingChild( pszFileName ) );
   RETURN_ON_FAIL( xmlTestSimpleArray( pszFileName ) );
   RETURN_ON_FAIL( xmlTestArrayWithSubTableAndSibling( pszFileName ) );
   RETURN_ON_FAIL( xmlTestWriteSimpleLayer( pszFileName ) );

#undef PRINTF_TEST
   DBG_PRINTF( "All [%u] tests successfully passed", s_ulTestCount );
   
   return NO_ERROR;
}
