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

typedef struct
{
   char* pszItem;
   BLOG_POST sPost;
} XML_ITEMS;
ERROR_CODE ReadFeedXmlFile( void );

#endif
