/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/
#ifndef XMLWRAPPER_H
#define XMLWRAPPER_H

#include <stdbool.h>
#include <stddef.h>
#include "Utils.h"

/* 
    Type of xml items for classification purposes
    Currently each item & subitems will only return strings
 */
typedef enum
{
    /* 
        Simplest XML Items. 
        Eg:
        <tag>STRING</tag>
     */
    XML_CHILD_STRING,
    /* 
        Nesting multiple XML Child Strings
        Eg:
        <tag>
            <nested_tag>STRING</nested_tag>
            <nested_tag2>STRING</nested_tag>
        </tag>
     */
    XML_TABLE,
    /* 
        Expansion of XML_TABLE such that multiple TABLES/Child Strings can be present
        Eg:
        <index>
            ...
        </index>
        <index>
            ...
        </index>
        ...
     */
    XML_SUB_ARRAY
} XML_TYPES;

/* 
    Structure for each XML item
    Currently will only fill in Strings so make sure you are only expecting strings
 */
typedef struct
{
    // Name of the element in the xml
    char *pszElementName;
    // Type of the element
    XML_TYPES eType;
    // Offset in the output structure
    uint32_t ulMemberOffset;
    // Sizeof the output variable
    uint32_t ulBufferSize;
    // Only applicable for XML_TABLE, XML_SUB_TABLE, pointer to the table containing XML_STRs
    const void *pavSubItem;
    // Only applicable for XML_TABLE number of XML_ITEMS in pvSubItem;
    uint32_t ulArrayElements;
    // Only applicable for XML_SUB_TABLE
    uint32_t ulArraySize;
} XML_ITEM;

#define XML_STR(element, structure, var)                                                                 \
    {                                                                                                    \
        element, XML_CHILD_STRING, offsetof(structure, var), sizeof(((structure *)0)->var), _null_, 0, 0 \
    }
#define XML_SUB_TABLE(element, structure, var, subItem, numOfElements)                                         \
    {                                                                                                          \
        element, XML_TABLE, offsetof(structure, var), sizeof(((structure *)0)->var), subItem, numOfElements, 0 \
    }

#define XML_ARRAY(element, structure, var, subItem, numOfElements, arraySize)                                              \
    {                                                                                                                      \
        element, XML_SUB_ARRAY, offsetof(structure, var), sizeof(((structure *)0)->var), subItem, numOfElements, arraySize \
    }

/* 
    Parse an XML file & populate XML_Items
    @param(INPUT):      pszFileName     -> Filename of the XML file to be parsed
    @param(INPUT):      pasItems        -> Array of XML Items expected by the app
    @param(INPUT):      ulArraySize     -> Number of items in pasItems
    @param(OUTPUT):     pvOutputStruct  -> The structure into which XML_ITEMS are gonna be populated
    @return:            NO_ERROR        -> Successful parsing
    @return:            INVALID_ARG     -> One or more parameters is null
 */
ERROR_CODE xmlWrapperParseFile(const char *pszFileName, const XML_ITEM *pasItems, uint32_t ulArraySize, void *pvOutputStruct);

/* 
    Write/Overwrite an XML file by using the  XML_Items
    @param(INPUT):      pszFileName     -> Filename of the XML file to be written
    @param(INPUT):      pasItems        -> Array of XML Items supplied by the app
    @param(INPUT):      ulArraySize     -> Number of items in pasItems
    @param(INPUT):      pvInputStruct   -> The structure from which XML_ITEMS are gonna be extracted
    @return:            NO_ERROR        -> Successful parsing
    @return:            INVALID_ARG     -> One or more parameters is null
 */
ERROR_CODE xmlWrapperWriteFile(const char *pszFileName, const XML_ITEM *pasItems, uint32_t ulArraySize, const void *pvInputStruct);

/* 
    Unit tests for XML Parsing
    @param:         NONE
    @return:        NO_ERROR    -> Success
    @return:        FAILED      -> One or more Unit Test failed
 */
ERROR_CODE XmlTest(void);

#endif