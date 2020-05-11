/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef DATABASE_H
#define DATABASE_H

#include "Utils.h"
#include "xmlWrapper.h"

typedef struct
{
   char* pszTitle;
   char* pszLink;
   char* pszCategory[3];
   char* pszDescription;
} BLOG_POST;

ERROR_CODE ReadFeedXmlFile( void );

#endif
