/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef CURL_WRAPPER_H
#define CURL_WRAPPER_H

#include "Utils.h"

/* 
    Curl Wrapper to download a URL 
    @param pszUrl: URL CURL calls & downloads
    @return NO_ERROR: Success
 */
ERROR_CODE DownloadFeedFile( const char * pszURL );

#endif
