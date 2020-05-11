/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/
#ifndef XMLWRAPPER_H
#define XMLWRAPPER_H

#include <stdbool.h>

#include "Utils.h"

typedef void *xmlWrapperPtr;

ERROR_CODE OpenXmlFile( xmlWrapperPtr *ppsFilePtr, const char *pszFilename );
ERROR_CODE WriteXml( const char * pszFilename, const UTIL_STR_ARRAY * psConfigKeys, const UTIL_STR_ARRAY * psConfigValues );
ERROR_CODE FindElement( const xmlWrapperPtr psFilePtr, const char *pszElementName, bool *pbFound );
ERROR_CODE ExtractDataFromElement( const xmlWrapperPtr psFilePtr, const char *pszElementName, char *pszDataBuffer, uint32_t ulBufferLen );
ERROR_CODE CleanupDumpXmlMemory( void );

#endif