/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/
#ifndef XMLWRAPPER_H
#define XMLWRAPPER_H

#include <stdbool.h>

#include "Utils.h"

typedef void *xmlWrapperPtr;
typedef void *xmlDocWriterPtr;
typedef void *xmlRootNodePtr;

typedef struct
{
   xmlDocWriterPtr ppsDocPtr;
   xmlRootNodePtr ppsRootNodePtr;
} xmlWriterPtrs;


ERROR_CODE OpenXmlFile( xmlWrapperPtr *ppsFilePtr, const char *pszFilename );
ERROR_CODE FindElement( const xmlWrapperPtr psFilePtr, const char *pszElementName, bool *pbFound );
ERROR_CODE ExtractDataFromElement( const xmlWrapperPtr psFilePtr, const char *pszElementName, char *pszDataBuffer, uint32_t ulBufferLen );
ERROR_CODE CleanupDumpXmlMemory( void );
ERROR_CODE CreateDocPtr( xmlWriterPtrs *psXmlFile );
ERROR_CODE CreateXmlNode( const xmlWriterPtrs *psXmlFile, const char *pszElement, const char *pszData );
ERROR_CODE WriteXmlFile( const xmlWriterPtrs *psXmlFile, const char *pszFilename );
#endif