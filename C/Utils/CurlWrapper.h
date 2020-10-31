/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef CURL_WRAPPER_H
#define CURL_WRAPPER_H

#include "Utils.h"

/* 
    Curl Wrapper to download a URL 
    @param pszUrl[IN]: URL CURL calls & downloads
    @param pszFilename[IN]: Filename to used for downloaded file
    @return NO_ERROR: Success
 */
ERROR_CODE DownloadFeedFile( const char * pszURL, const char *pszFilename );

#endif
