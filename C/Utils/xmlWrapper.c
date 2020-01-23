
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

ERROR_CODE ReadXml( const char *pszFilename, const UTIL_STR_ARRAY *psKeys, UTIL_STR_ARRAY *psStrArray )
{
  ERROR_CODE eRet = NO_ERROR;
  xmlTextReaderPtr pReader = NULL;
  uint32_t x = 0;

  if( pszFilename == NULL || psKeys == NULL || psStrArray == NULL )
  {
    return INVALID_ARG;
  }

  memset( psStrArray, 0, sizeof( UTIL_STR_ARRAY ));
  
  pReader = xmlReaderForFile(pszFilename, NULL, 0 );
  if (pReader != NULL) 
  {
    while (xmlTextReaderRead(pReader) == 1 && x < CONFIG_LAST ) 
    {
        const xmlChar *pName = xmlTextReaderConstName(pReader);
        if( pName != NULL && ( strcmp( psKeys->aszStringArray[x], pName) == 0 ) )
        {
          xmlTextReaderRead(pReader);
          const xmlChar *pValue = xmlTextReaderConstValue(pReader);
          strcpy( psStrArray->aszStringArray[x], pValue );
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