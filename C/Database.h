/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#ifndef DATABASE_H
#define DATABASE_H

#include "Utils.h"
#include "xmlWrapper.h"

/*
    Blog Post Structure
    - Valid Blog post: Title & Link cannot be empty
    Times share can be empty
    Assumption: There is only website being used to share posts.
*/
typedef struct
{
    char szTitle[128 + 1];     // Title extracted from the website's RSS
    char szLink[128 + 1];      // Link extracted from the website's RSS
    char szTimesShared[2 + 1]; // Non-RSS variable. Used for internal database
} BLOG_POST;

/*
    Initializes Database variables
    - Will try to open the database file
    - If database file is absent, will try to open the RSS file
    - Name of the RSS file is in Config file
    @param: None
    @return: NO_ERROR = Success
*/
ERROR_CODE Database_Init(void);

ERROR_CODE Database_GetOldestLeastSharedPost(BLOG_POST *psPost);
ERROR_CODE Database_Tests(void);
ERROR_CODE Database_AddNewItem(const BLOG_POST *psPost);
bool Database_IsUniquePost(const BLOG_POST *psPost);

#endif
