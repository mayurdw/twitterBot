/*
    Author:  Mayur Wadhwani
    Created: Feb 2020
*/

#include "config.h"

static const UTIL_STR_ARRAY s_sConfigKeys = { "currentFilename", "daysToFileUpdate" };
static UTIL_STR_ARRAY s_sConfig = { 0, };

static void DebugConfig(void);

ERROR_CODE ReadConfig(void)
{
    ERROR_CODE eRet = ReadXml(CONFIG_FILENAME, &s_sConfigKeys, &s_sConfig);

    DebugConfig();
    return eRet;
}

static void DebugConfig(void)
{
    printf("%s: Debugging config\n", __func__);
    for (CONFIG_KEYS x = 0; x < CONFIG_LAST; x++)
    {
        printf("s_sConfig.aszStringArray[%d] = %s\n", x, s_sConfig.aszStringArray[x]);
    }
}

#if 0
static ERROR_CODE InitConfig(void)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL;/* node pointers */
    char buff[256] = { 0, };
    int i = 0, j = 0;
    char szFilename[MAX_FILENAME_LEN + 1] = { 0, };

    GenerateFileName(szFilename, sizeof(szFilename));

    LIBXML_TEST_VERSION;

    /*
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(doc, root_node);

    /*
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node.
     */
    xmlNewChild(root_node, NULL, BAD_CAST s_apszConfigList[0], BAD_CAST szFilename);
    strcpy(s_aszConfigs[0], szFilename);
    xmlNewChild(root_node, NULL, BAD_CAST s_apszConfigList[1], BAD_CAST DAYS_UNTIL_NEXT_UPDATE);
    strcpy(s_aszConfigs[1], DAYS_UNTIL_NEXT_UPDATE);
    /*
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc(CONFIG_FILENAME, doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return NO_ERROR;
}
#endif