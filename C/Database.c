/*
    Author: Mayur Wadhwani
    Created: Feb 2020
*/

#include "Database.h"
#include "config.h"

// Macros
#define DATABASE_FORMAT "%s,%s,%s,%s,%s,%s,%i\n"
#define DATABASE_FILE   "database"
#define MAX_BLOG_POSTS  200
#define LINE_LEN        ( sizeof( BLOG_POST ) )

// Static variables
static BLOG_POST s_asList[MAX_BLOG_POSTS] = { 0, };
static xmlWrapperPtr psTextReader = _null_;

// Static functions
static ERROR_CODE CreateDatabaseFile( void );
static ERROR_CODE ExtractAndPopulate( const char *pszElementName, char *pszBufferToPopulate, uint32_t ulBufferSize );
static ERROR_CODE ReadDatabaseFile( void );
static ERROR_CODE ExtractFromLine( const char *pszTemp, BLOG_POST *psPost );

static ERROR_CODE ExtractAndPopulate( const char *pszElementName, char *pszBufferToPopulate, uint32_t ulBufferSize )
{
   char szTemp[1024 + 1] = { 0, };

   RETURN_ON_NULL( pszElementName );
   RETURN_ON_NULL( psTextReader );
   UTIL_ASSERT( ulBufferSize > 0, INVALID_ARG );

   RETURN_ON_FAIL( ExtractDataFromElement( psTextReader, pszElementName, szTemp, sizeof( szTemp ) ) );
   if( strlen( szTemp ) == 0 )
   {
      RETURN_ON_FAIL( Strcpy_safe( pszBufferToPopulate, " ", ulBufferSize ) );
   }
   else
   {
      RETURN_ON_FAIL( Strcpy_safe( pszBufferToPopulate, szTemp, ulBufferSize ) );
   }


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
 
   memset( &s_asList, 0, sizeof( s_asList ) );

   RETURN_ON_FAIL( Config_GetRssFilename( szFilename, sizeof( szFilename ) ) );

   RETURN_ON_FAIL( OpenXmlFile( &psTextReader, szFilename ) );

   while( y < MAX_BLOG_POSTS )
   {
      eRet = FindElement( psTextReader, "item", &bFound );
      if( ISERROR( eRet ) )
         break;
      if( bFound )
      {
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[0], s_asList[y].szTitle, sizeof( s_asList[y].szTitle ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[1], s_asList[y].szLink, sizeof( s_asList[y].szLink ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[2], s_asList[y].aszCategory[0], sizeof( s_asList[y].aszCategory[0] ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[3], s_asList[y].aszCategory[1], sizeof( s_asList[y].aszCategory[1] ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[4], s_asList[y].aszCategory[2], sizeof( s_asList[y].aszCategory[2] ) ) );
         RETURN_ON_FAIL( ExtractAndPopulate( apszItemList[5], s_asList[y].szDescription, sizeof( s_asList[y].szDescription ) ) );

      }

      eRet = FindElement( psTextReader, "item", &bFound );
      if( ISERROR( eRet ) )
         break;
      y++;
   }


   return NO_ERROR;
}

ERROR_CODE Database_Init( bool bReadXmlFile )
{
   if( bReadXmlFile )
   {
      ReadFeedXmlFile();
      CreateDatabaseFile();
   }
   else
   {
      ReadDatabaseFile();
   }

   return NO_ERROR;
}

ERROR_CODE CreateDatabaseFile( void )
{
   FILE *pFile = _null_;
   int x = 0;

   pFile = fopen( DATABASE_FILE, "wb" );

   while( ( x < MAX_BLOG_POSTS ) && ( strlen( s_asList[x].szTitle ) > 0 ) )
   {
      fprintf( pFile, DATABASE_FORMAT, s_asList[x].szTitle, s_asList[x].szLink, s_asList[x].aszCategory[0], s_asList[x].aszCategory[1], s_asList[x].aszCategory[2], s_asList[x].szDescription, s_asList[x].iNumOfTimesShared );
      x++;
   }

   fclose( pFile );

   return NO_ERROR;
}

ERROR_CODE ReadDatabaseFile( void )
{
   ERROR_CODE eRet = NO_ERROR;
   FILE *pFile = _null_;

   memset( s_asList, 0, sizeof( s_asList ) );

   pFile = fopen( DATABASE_FILE, "rb" );
   if( pFile == _null_ )
   {
      eRet = FILE_ERROR;
   }
   else
   {
      uint32_t x = 0;
      int y = 0;
      char szTemp[LINE_LEN + 1] = { 0, };
      char *pszRet = " ";

      while( ( pszRet != _null_ ) && ( x < MAX_BLOG_POSTS ) )
      {
         pszRet = fgets( szTemp, sizeof( szTemp ), pFile );
         if( pszRet != _null_ )
         {
            printf( "line = %s\n", szTemp );
            RETURN_ON_FAIL( ExtractFromLine( szTemp, &s_asList[x] ) );

            //printf( DATABASE_FORMAT, s_asList[x].szTitle, s_asList[x].szLink, s_asList[x].aszCategory[0], s_asList[x].aszCategory[1], s_asList[x].aszCategory[2], s_asList[x].szDescription, s_asList[x].iNumOfTimesShared );
         }

         x++;
      }

      fclose( pFile );
   }

   return NO_ERROR;
}

ERROR_CODE ExtractFromLine( const char *pszTemp, BLOG_POST *psPost )
{
   char szTemp[LINE_LEN + 1] = { 0, };
   char *pszToken = ",", *pszCurrentPosition = _null_;

   RETURN_ON_NULL( pszTemp );
   RETURN_ON_NULL( psPost );
   memset( psPost, 0, sizeof( BLOG_POST ) );

   Strcpy_safe( szTemp, pszTemp, sizeof( szTemp ) );

#define TOKENIZE( x, y ) \
   {\
      if( pszCurrentPosition != _null_ ) \
      { \
         pszCurrentPosition = strtok( x, pszToken ); \
         if( pszCurrentPosition != _null_ ) \
         { \
            printf( "pszCurrentPosition = %s", pszCurrentPosition ); \
            Strcpy_safe( y, pszCurrentPosition, sizeof( y ) ); \
         }\
      } \
   } \

   TOKENIZE( szTemp, psPost->szTitle );
   printf( "here\n" );
   TOKENIZE( _null_, psPost->szLink );
   printf( "here\n" );
   TOKENIZE( _null_, psPost->aszCategory[0] );
   printf( "here\n" );
   TOKENIZE( _null_, psPost->aszCategory[1] );
   printf( "here\n" );
   TOKENIZE( _null_, psPost->aszCategory[2] );
   printf( "here\n" );
   TOKENIZE( _null_, psPost->szDescription );
   printf( "here\n" );

   pszCurrentPosition = strtok( _null_, pszToken );
   psPost->iNumOfTimesShared = atoi( pszCurrentPosition );
   printf( "End\n" );


   return NO_ERROR;
}
