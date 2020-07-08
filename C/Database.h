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
    char szTitle[128 + 1];
    char szLink[128 + 1];
    // non-xml variable
    int iNumOfTimesShared;
} BLOG_POST;

ERROR_CODE Database_Init(void);

#endif
