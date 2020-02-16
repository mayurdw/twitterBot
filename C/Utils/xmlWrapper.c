/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

ERROR_CODE ReadXml(const char* pszFilename, const UTIL_STR_ARRAY* psKeys, UTIL_STR_ARRAY* psStrArray)
{
    ERROR_CODE eRet = NO_ERROR;
    xmlTextReaderPtr pReader = NULL;
    uint32_t x = 0;

    if (pszFilename == NULL || psKeys == NULL || psStrArray == NULL)
    {
        return INVALID_ARG;
    }

    memset(psStrArray, 0, sizeof(UTIL_STR_ARRAY));

    pReader = xmlReaderForFile(pszFilename, NULL, 0);
    if (pReader != NULL)
    {
        while (xmlTextReaderRead(pReader) == 1 && x < CONFIG_LAST)
        {
            const xmlChar* pName = xmlTextReaderConstName(pReader);
            if (pName != NULL && (strcmp(psKeys->aszStringArray[x], pName) == 0))
            {
                xmlTextReaderRead(pReader);
                const xmlChar* pValue = xmlTextReaderConstValue(pReader);
                strcpy(psStrArray->aszStringArray[x], pValue);
                x++;
            }
        }

        xmlFreeTextReader(pReader);
    }
    else
    {
        eRet = FILE_ERROR;
    }

    // /*
    //  * Cleanup function for the XML library.
    //  */
    xmlCleanupParser();
    // /*
    //  * this is to debug memory for regression tests
    //  */
    xmlMemoryDump();

    return eRet;
}

ERROR_CODE WriteXml( const char * pszFilename, const UTIL_STR_ARRAY * psConfigKeys, const UTIL_STR_ARRAY * psConfigValues )
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL;/* node pointers */
    char buff[256] = { 0, };
    int i = 0, j = 0;

    if( pszFilename == NULL || psConfigKeys == NULL || psConfigValues == NULL )
    {
        return INVALID_ARG;
    }

    LIBXML_TEST_VERSION;

    /*
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc( BAD_CAST "1.0" );
    root_node = xmlNewNode( NULL, BAD_CAST "root" );
    xmlDocSetRootElement( doc, root_node );

    /*
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node.
     */

    while( i < CONFIG_LAST && strlen( psConfigKeys->aszStringArray[i] ) > 0 && strlen( psConfigValues->aszStringArray[i] ) > 0 )
    {
        xmlNewChild( root_node, NULL, BAD_CAST psConfigKeys->aszStringArray[i], BAD_CAST psConfigValues->aszStringArray[i] );
        i++;
    }

    /*
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc( pszFilename, doc, "UTF-8", 1 );

    /*free the document */
    xmlFreeDoc( doc );

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return NO_ERROR;
}
