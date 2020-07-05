/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/
#ifndef XMLWRAPPER_H
#define XMLWRAPPER_H

#include <stdbool.h>
#include <stddef.h>
#include "Utils.h"

typedef enum
{
    XML_ROOT = 0,
    XML_CHILD_STRING,
    XML_TABLE,
    XML_SUB_ARRAY
} XML_TYPES;

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
    // Only applicable for XML_TABLE, pointer to the table containing XML_STRs
    const void *pavSubItem;
    // Only applicable for XML_TABLE number of XML_ITEMS in pvSubItem;
    uint32_t ulArrayElements;
} XML_ITEM;

#define XML_STR(element, structure, var)                                                              \
    {                                                                                                 \
        element, XML_CHILD_STRING, offsetof(structure, var), sizeof(((structure *)0)->var), _null_, 0 \
    }
#define XML_SUB_TABLE(element, structure, var, subItem, numOfElements)                                      \
    {                                                                                                       \
        element, XML_TABLE, offsetof(structure, var), sizeof(((structure *)0)->var), subItem, numOfElements \
    }

// TODO: Add number of items
//#define XML_ARRAY(element, structure, var) element, offsetof(structure, var), sizeof(((structure *)0)->var), XML_SUB_ARRAY

typedef void *xmlWrapperPtr;
typedef void *xmlDocWriterPtr;
typedef void *xmlRootNodePtr;

typedef struct
{
    xmlDocWriterPtr ppsDocPtr;
    xmlRootNodePtr ppsRootNodePtr;
} xmlWriterPtrs;

ERROR_CODE OpenXmlFile(xmlWrapperPtr *ppsFilePtr, const char *pszFilename);
ERROR_CODE FindElement(const xmlWrapperPtr psFilePtr, const char *pszElementName, bool *pbFound);
ERROR_CODE ExtractDataFromElement(const xmlWrapperPtr psFilePtr, const char *pszElementName, char *pszDataBuffer, uint32_t ulBufferLen);
ERROR_CODE CleanupDumpXmlMemory(void);
ERROR_CODE CreateDocPtr(xmlWriterPtrs *psXmlFile);
ERROR_CODE CreateXmlNode(const xmlWriterPtrs *psXmlFile, const char *pszElement, const char *pszData);
ERROR_CODE WriteXmlFile(const xmlWriterPtrs *psXmlFile, const char *pszFilename);
ERROR_CODE XmlTest(void);

ERROR_CODE xmlWrapperParseFile(const char *pszFileName, const XML_ITEM *pasItems, uint32_t ulArraySize, void *pvOutputStruct);
// TODO: Figure out how to writeFile with the same method too
#endif