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
    XML_CHILD_STRING = 0,
    XML_TABLE,
    XML_SUB_ARRAY
} XML_TYPES;

typedef struct
{
    char *pszElementName;
    uint32_t ulMemberOffset;
    uint32_t ulBufferSize;
    XML_TYPES eType;
} XML_ITEM;

#define XML_STR(element, structure, var) element, offsetof(structure, var), sizeof(((structure *)0)->var), XML_CHILD_STRING
// TODO: Add table elements?
#define XML_SUB_TABLE(element, structure, var) element, offsetof(structure, var), sizeof(((structure *)0)->var), XML_TABLE
// TODO: Add number of items
#define XML_ARRAY(element, structure, var) element, offsetof(structure, var), sizeof(((structure *)0)->var), XML_SUB_ARRAY

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