
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

ERROR_CODE ReadXml( const char *pszFilename, const char **ppaszList, uint32_t ulArrayLen )
{
  ERROR_CODE eRet = NO_ERROR;
  xmlTextReaderPtr pReader = NULL;
  uint32_t x = 0;

  if( pszFilename == NULL || ppaszList == NULL || ulArrayLen == 0 )
  {
    return INVALID_ARG;
  }

  pReader = xmlReaderForFile(pszFilename, NULL, 0 );
  if (pReader != NULL) 
  {
    while (xmlTextReaderRead(pReader) == 1 && x < ulArrayLen ) 
    {
        const xmlChar *pName = xmlTextReaderConstName(pReader);

        if( pName != NULL && ( strcmp( ppaszList[x], pName) == 0 ) )
        {
          xmlTextReaderRead(pReader);
          const xmlChar *pValue = xmlTextReaderConstValue(pReader);
          //strcpy( s_aszConfigs[x], pValue );
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