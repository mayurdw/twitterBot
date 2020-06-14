/*
    Author:  Mayur Wadhwani
    Created: January 2020
*/

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "xmlWrapper.h"

#define DEBUG(x) printf(x)

   /*
    * A person record
    * an xmlChar * is really an UTF8 encoded char string (0 terminated)
    */
typedef struct person
{
   xmlChar *name;
   xmlChar *email;
   xmlChar *company;
   xmlChar *organisation;
   xmlChar *smail;
   xmlChar *webPage;
   xmlChar *phone;
} person, *personPtr;

/*
 * And the code needed to parse it
 */
static personPtr
parsePerson( xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur )
{
   personPtr ret = NULL;

   DEBUG( "parsePerson\n" );
   /*
    * allocate the struct
    */
   ret = ( personPtr )malloc( sizeof( person ) );
   if( ret == NULL )
   {
      DBG_PRINTF( "out of memory\n" );
      return( NULL );
   }
   memset( ret, 0, sizeof( person ) );

   /* We don't care what the top level element name is */
   /* COMPAT xmlChildrenNode is a macro unifying libxml1 and libxml2 names */
   cur = cur->xmlChildrenNode;
   while( cur != NULL )
   {
      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Person" ) ) &&
         ( cur->ns == ns ) )
         ret->name = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1 );
      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Email" ) ) &&
         ( cur->ns == ns ) )
         ret->email = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1 );
      cur = cur->next;
   }

   return( ret );
}

/*
 * and to print it
 */
static void
printPerson( personPtr cur )
{
   if( cur == NULL ) return;
   DBG_PRINTF( "------ Person\n" );
   if( cur->name ) DBG_PRINTF( "	name: %s\n", cur->name );
   if( cur->email ) DBG_PRINTF( "	email: %s\n", cur->email );
   if( cur->company ) DBG_PRINTF( "	company: %s\n", cur->company );
   if( cur->organisation ) DBG_PRINTF( "	organisation: %s\n", cur->organisation );
   if( cur->smail ) DBG_PRINTF( "	smail: %s\n", cur->smail );
   if( cur->webPage ) DBG_PRINTF( "	Web: %s\n", cur->webPage );
   if( cur->phone ) DBG_PRINTF( "	phone: %s\n", cur->phone );
   DBG_PRINTF( "------\n" );
}

/*
 * a Description for a Job
 */
typedef struct job
{
   xmlChar *projectID;
   xmlChar *application;
   xmlChar *category;
   personPtr contact;
   int nbDevelopers;
   personPtr developers[100]; /* using dynamic alloc is left as an exercise */
} job, *jobPtr;

/*
 * And the code needed to parse it
 */
static jobPtr
parseJob( xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur )
{
   jobPtr ret = NULL;

   DEBUG( "parseJob\n" );
   /*
    * allocate the struct
    */
   ret = ( jobPtr )malloc( sizeof( job ) );
   if( ret == NULL )
   {
      DBG_PRINTF( "out of memory\n" );
      return( NULL );
   }
   memset( ret, 0, sizeof( job ) );

   /* We don't care what the top level element name is */
   cur = cur->xmlChildrenNode;
   while( cur != NULL )
   {

      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Project" ) ) &&
         ( cur->ns == ns ) )
      {
         ret->projectID = xmlGetProp( cur, ( const xmlChar * )"ID" );
         if( ret->projectID == NULL )
         {
            DBG_PRINTF( "Project has no ID\n" );
         }
      }
      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Application" ) ) &&
         ( cur->ns == ns ) )
         ret->application =
         xmlNodeListGetString( doc, cur->xmlChildrenNode, 1 );
      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Category" ) ) &&
         ( cur->ns == ns ) )
         ret->category =
         xmlNodeListGetString( doc, cur->xmlChildrenNode, 1 );
      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Contact" ) ) &&
         ( cur->ns == ns ) )
         ret->contact = parsePerson( doc, ns, cur );
      cur = cur->next;
   }

   return( ret );
}

/*
 * and to print it
 */
static void
printJob( jobPtr cur )
{
   int i;

   if( cur == NULL ) return;
   DBG_PRINTF( "=======  Job\n" );
   if( cur->projectID != NULL ) DBG_PRINTF( "projectID: %s\n", cur->projectID );
   if( cur->application != NULL ) DBG_PRINTF( "application: %s\n", cur->application );
   if( cur->category != NULL ) DBG_PRINTF( "category: %s\n", cur->category );
   if( cur->contact != NULL ) printPerson( cur->contact );
   DBG_PRINTF( "%d developers\n", cur->nbDevelopers );

   for( i = 0; i < cur->nbDevelopers; i++ ) printPerson( cur->developers[i] );
   DBG_PRINTF( "======= \n" );
}

/*
 * A pool of Gnome Jobs
 */
typedef struct gjob
{
   int nbJobs;
   jobPtr jobs[500]; /* using dynamic alloc is left as an exercise */
} gJob, *gJobPtr;


static gJobPtr
parseGjobFile( char *filename ATTRIBUTE_UNUSED )
{
   xmlDocPtr doc;
   gJobPtr ret;
   jobPtr curjob;
   xmlNsPtr ns;
   xmlNodePtr cur;

   /*
    * build an XML tree from a the file;
    */
   doc = xmlParseFile( filename );
   if( doc == NULL ) return( NULL );

   /*
    * Check the document is of the right kind
    */

   cur = xmlDocGetRootElement( doc );
   if( cur == NULL )
   {
      DBG_PRINTF( "empty document\n" );
      xmlFreeDoc( doc );
      return( NULL );
}
   ns = xmlSearchNsByHref( doc, cur,
      ( const xmlChar * )"http://www.gnome.org/some-location" );
   if( ns == NULL )
   {
      DBG_PRINTF(          "document of the wrong type, GJob Namespace not found\n" );
      xmlFreeDoc( doc );
      return( NULL );
   }
   if( xmlStrcmp( cur->name, ( const xmlChar * )"Helping" ) )
   {
      DBG_PRINTF( "document of the wrong type, root node != Helping" );
      xmlFreeDoc( doc );
      return( NULL );
   }

   /*
    * Allocate the structure to be returned.
    */
   ret = ( gJobPtr )malloc( sizeof( gJob ) );
   if( ret == NULL )
   {
      DBG_PRINTF( "out of memory\n" );
      xmlFreeDoc( doc );
      return( NULL );
   }
   memset( ret, 0, sizeof( gJob ) );

   /*
    * Now, walk the tree.
    */
    /* First level we expect just Jobs */
   cur = cur->xmlChildrenNode;
   while( cur && xmlIsBlankNode( cur ) )
   {
      cur = cur->next;
   }
   if( cur == 0 )
   {
      xmlFreeDoc( doc );
      free( ret );
      return ( NULL );
   }
   if( ( xmlStrcmp( cur->name, ( const xmlChar * )"Jobs" ) ) || ( cur->ns != ns ) )
   {
      DBG_PRINTF( "document of the wrong type, was '%s', Jobs expected",
         cur->name );
      DBG_PRINTF( "xmlDocDump follows\n" );
#ifdef LIBXML_OUTPUT_ENABLED
      //xmlDocDump( doc );
      DBG_PRINTF( "xmlDocDump finished\n" );
#endif /* LIBXML_OUTPUT_ENABLED */
      xmlFreeDoc( doc );
      free( ret );
      return( NULL );
   }

   /* Second level is a list of Job, but be laxist */
   cur = cur->xmlChildrenNode;
   while( cur != NULL )
   {
      if( ( !xmlStrcmp( cur->name, ( const xmlChar * )"Job" ) ) &&
         ( cur->ns == ns ) )
      {
         curjob = parseJob( doc, ns, cur );
         if( curjob != NULL )
            ret->jobs[ret->nbJobs++] = curjob;
         if( ret->nbJobs >= 500 ) break;
      }
      cur = cur->next;
   }

   return( ret );
}

static void
handleGjob( gJobPtr cur )
{
   int i;

   /*
    * Do whatever you want and free the structure.
    */
   DBG_PRINTF( "%d Jobs registered\n", cur->nbJobs );
   for( i = 0; i < cur->nbJobs; i++ ) printJob( cur->jobs[i] );
}


ERROR_CODE XmlTest( void )
{
   int i;
   gJobPtr cur;

   /* COMPAT: Do not generate nodes for formatting spaces */
   LIBXML_TEST_VERSION
      xmlKeepBlanksDefault( 0 );

   cur = parseGjobFile( "test.xml" );
   if( cur )
      handleGjob( cur );
   else
      DBG_PRINTF( "Error parsing file 'test.xml'\n" );

   /* Clean up everything else before quitting. */
   xmlCleanupParser();

   return( 0 );
}

ERROR_CODE OpenXmlFile( xmlWrapperPtr *ppsFilePtr, const char* pszFilename )
{
   xmlTextReaderPtr pReader = _null_;

   RETURN_ON_NULL( ppsFilePtr );
   RETURN_ON_NULL( pszFilename );
   UTIL_ASSERT( strlen( pszFilename ) > 0, INVALID_ARG );

   if( *ppsFilePtr != _null_ )
   {
      CleanupDumpXmlMemory( );
      *ppsFilePtr = _null_;
   }

   pReader = xmlReaderForFile( pszFilename, _null_, 0 );
   RETURN_ON_NULL( pReader );

   *ppsFilePtr = ( xmlWrapperPtr * )pReader;

   return NO_ERROR;
}


ERROR_CODE FindElement( const xmlWrapperPtr psFilePtr, const char *pszElementName, bool *pbFound )
{
   xmlTextReaderPtr psReader = 0;
   bool bFound = false;

   RETURN_ON_NULL( psFilePtr );
   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( pbFound );
   *pbFound = false;
   psReader = ( xmlTextReaderPtr )psFilePtr;

   while( !bFound && ( xmlTextReaderRead( psReader ) == 1 ) )
   {
      const xmlChar *pszName = xmlTextReaderConstName( psReader );
      if( pszName != _null_ )
      {
         bFound = ( strcmp( pszName, pszElementName ) == 0 );
      }
   }

   *pbFound = bFound;

   return bFound ? NO_ERROR : NOT_FOUND;
}

ERROR_CODE ExtractDataFromElement( const xmlWrapperPtr psFilePtr, const char* pszElementName, char* pszDataBuffer, uint32_t ulBufferLen )
{
   bool bFound = false;
   xmlChar* pszValue = _null_;
   xmlTextReaderPtr psReader = 0;

   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( pszDataBuffer );
   UTIL_ASSERT( strlen( pszElementName ) > 0, INVALID_ARG );
   UTIL_ASSERT( ulBufferLen > 0, INVALID_ARG );
   RETURN_ON_NULL( psFilePtr );

   memset( pszDataBuffer, 0, ulBufferLen );
   psReader = ( xmlTextReaderPtr )psFilePtr;
   RETURN_ON_FAIL( FindElement( psFilePtr, pszElementName, &bFound ) );

   if( bFound )
   {
      // We have the first tag
      xmlTextReaderRead( psReader );
      Strcpy_safe( pszDataBuffer, xmlTextReaderConstValue( psReader ), ulBufferLen );
      // this should close the tag
      xmlTextReaderRead( psReader );
   }

   return NO_ERROR;
}

ERROR_CODE CleanupDumpXmlMemory( void )
{
   // /*
       //  * Cleanup function for the XML library.
       //  */
   xmlCleanupParser( );
   // /*
   //  * this is to debug memory for regression tests
   //  */
   xmlMemoryDump( );

}

ERROR_CODE CreateDocPtr( xmlWriterPtrs *psXmlFile )
{
   xmlDocPtr psDoc = _null_;       /* Document pointer */
   xmlNodePtr root_node = _null_;  /* node pointers */
 
   RETURN_ON_NULL( psXmlFile );

   LIBXML_TEST_VERSION;

   /*
    * Creates a new Document, a node and set it as a root node
    */
   psDoc = xmlNewDoc( BAD_CAST "1.0" );
   root_node = xmlNewNode( _null_, BAD_CAST "root" );
   xmlDocSetRootElement( psDoc, root_node );

   psXmlFile->ppsDocPtr = ( xmlDocWriterPtr* )psDoc;
   psXmlFile->ppsRootNodePtr = ( xmlRootNodePtr* )root_node;

   return NO_ERROR;
}

ERROR_CODE CreateXmlNode( const xmlWriterPtrs *psXmlFile, const char *pszElement, const char *pszData )
{
   xmlNodePtr root_node = _null_;

   RETURN_ON_NULL( psXmlFile );
   RETURN_ON_NULL( psXmlFile->ppsDocPtr );
   RETURN_ON_NULL( psXmlFile->ppsRootNodePtr );
   RETURN_ON_NULL( pszElement );
   RETURN_ON_NULL( pszData );

   root_node = ( xmlNodePtr )psXmlFile->ppsRootNodePtr;

   xmlNewChild( root_node, _null_, BAD_CAST pszElement, BAD_CAST pszData );

   return NO_ERROR;
}

ERROR_CODE WriteXmlFile( const xmlWriterPtrs *psXmlFile, const char *pszFilename )
{
   xmlDocPtr psDoc = _null_;

   RETURN_ON_NULL( psXmlFile );
   RETURN_ON_NULL( psXmlFile->ppsDocPtr );
   RETURN_ON_NULL( pszFilename );

   psDoc = ( xmlDocPtr )psXmlFile->ppsDocPtr;

   xmlSaveFormatFileEnc( pszFilename, psDoc, "UTF-8", 1 );
   CleanupDumpXmlMemory();

   return NO_ERROR;
}
