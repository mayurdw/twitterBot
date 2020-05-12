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
   char szTitle[128+1];
   char szLink[128+1];
   char aszCategory[3][32+1];
   char szDescription[1024+1];
} BLOG_POST;

ERROR_CODE ReadFeedXmlFile( void );

#endif
