/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

// Static variables
static BLOG_POST s_asList[200] = { 0, };
static xmlWrapperPtr psTextReader = _null_;

// Static functions
static ERROR_CODE ReadDatabaseFile( void );
static ERROR_CODE ExtractAndPopulate( const char *pszElementName, char *pszBufferToPopulate, uint32_t ulBufferSize );

static ERROR_CODE ExtractAndPopulate( const char *pszElementName, char *pszBufferToPopulate, uint32_t ulBufferSize )
{
   char szTemp[1024 + 1] = { 0, };

   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( psTextReader );
   UTIL_ASSERT( ulBufferSize > 0, INVALID_ARG );

   RETURN_ON_FAIL( ExtractDataFromElement( psTextReader, pszElementName, szTemp, sizeof( szTemp ) ) );
   RETURN_ON_FAIL( Strcpy_safe( pszBufferToPopulate, szTemp, ulBufferSize ) );

   return NO_ERROR;
}

ERROR_CODE ReadFeedXmlFile( void )
{
   const char *apszItemList[] = { "title", "link", "category", "category", "category", "description" };
   char szFilename[MAX_FILENAME_LEN + 1] = { 0, };
   char szTemp[4096 + 1] = { 0, };
   ERROR_CODE eRet = NO_ERROR;
   bool bFound = false;
   int y = 0;
   BLOG_POST asPost[150] = { 0, };

   RETURN_ON_FAIL( GetConfig( CONFIG_CURRENT_FILENAME, szFilename, sizeof( szFilename ) ) );

   RETURN_ON_FAIL( OpenXmlFile( &psTextReader, szFilename ) );

   while( y < 150 )
   {
      eRet = FindElement( psTextReader, "item", &bFound );
      if( ISERROR( eRet ) )
         break;
      if( bFound )
      {
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[0], asPost[y].szTitle, sizeof( asPost[y].szTitle ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[1], asPost[y].szLink, sizeof( asPost[y].szLink ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[2], asPost[y].aszCategory[0], sizeof( asPost[y].aszCategory[0] ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[3], asPost[y].aszCategory[1], sizeof( asPost[y].aszCategory[1] ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[4], asPost[y].aszCategory[2], sizeof( asPost[y].aszCategory[2] ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[5], asPost[y].szDescription, sizeof( asPost[y].szDescription ) ) );

      }

      eRet = FindElement( psTextReader, "item", &bFound );
      if( ISERROR( eRet ) )
         break;
      y++;
   }

   return NO_ERROR;
}

ERROR_CODE ReadDatabaseFile( void )
{
   return NO_ERROR;
}
