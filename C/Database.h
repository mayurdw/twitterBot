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
    Will try to open the database file
    If database file is absent, will try to open the RSS file
    Name of the RSS file is in Config file
    @param: None
    @return: NO_ERROR = Success
*/
ERROR_CODE Database_Init(void);

/* 
    Gets the blog post which has been shared the least number of times
    When searching for the post, it will try to find the post which is at a higher index in the array
    @param (OUTPUT):    psPost      -> Blog Post shared least number of times
    @return:            NO_ERROR    -> Success
 */
ERROR_CODE Database_GetOldestLeastSharedPost(BLOG_POST *psPost);

/* 
    Adds new blog post to the database.
    Will always add a post to index 0 of the queue
    @param (INPUT):     psPost      -> New Blog post which needs to be added
    @return:            NO_ERROR    -> Success
    @return:            INVALID_ARG -> psPost pointer is NULL
    @return:            OVERFLOW    -> Database is full, need to expand the count
 */
ERROR_CODE Database_AddNewItem(const BLOG_POST *psPost);

/* 
    Compares blog post with the database to find if the post is unique
    @param (INPUT):     psPost      -> Blog post whose uniquesness is to be determined
    @return:            true        -> Blog post is unique
    @return:            false       -> Blog post is already in the database
 */
bool Database_IsUniquePost(const BLOG_POST *psPost);

/*
    Updates a post which is already on the database.
    @param (INPUT):     psPost      -> Blog Post which needs to be updated
    @return:            NO_ERROR    -> Success
    @return:            INVALID_ARG -> psPost is invalid
    @return:            OVERFLOW    -> Database is full, need to expand the count
*/
ERROR_CODE Database_UpdateTimesShared( const BLOG_POST *psPost );

/* 
    Refreshes already initialized database
    Will re-read the config specified RSS file
    @return             NO_ERROR    -> Database updated
 */
ERROR_CODE Database_RefreshDatabase( void );

/* 
    Database Unit Tests
    @param:             NONE
    @return:            NO_ERROR    -> All unit tests pass
    @return:            FAILED      -> One or more Unit test failed. Enabled Database debugging for more information
 */
ERROR_CODE Database_Tests(void);

#endif
