/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/
#ifndef XMLWRAPPER_H
#define XMLWRAPPER_H

#include "Utils.h"

ERROR_CODE ReadXml(const char* pszFilename, const UTIL_STR_ARRAY* psKeys, UTIL_STR_ARRAY* psStrArray);
ERROR_CODE WriteXml( const char * pszFilename, const UTIL_STR_ARRAY * psConfigKeys, const UTIL_STR_ARRAY * psConfigValues );

#endif