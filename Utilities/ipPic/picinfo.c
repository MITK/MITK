
/*
 * $RCSfile$
 *---------------------------------------------------------------
 * DESCRIPTION
 *   prints information about a picfile
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:10:00  andre
 * Initial revision
 *
 * Revision 0.1  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision, NO error checking
 *
 *
 *---------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)picinfo \t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

/*
** defines
*/

/*
** standard includes
*/
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <limits.h>

/*
** ip includefiles
*/
#include "ipPic.h"

/*
** typedefs
*/
typedef int Bool;
#define TRUE 1
#define FALSE 0

/*
** private macros
*/
#define Min(x, y) (((x) < (y)) ? (x) : (y))
#define Max(x, y) (((x) > (y)) ? (x) : (y))


#define MIN_MAX(min, max, type)            \
  for( i=0; i<_ipPicElements(pic); i++ )   \
    {                                      \
      type pixel = ((type *)pic->data)[i]; \
                                           \
      if( pixel < min )                    \
        min = pixel;                       \
      else if( pixel > max )               \
        max = pixel;                       \
    }

/*
** the action starts here
*/
int main( int argc, char *argv[] )
{
  int i;

  long min_i = LONG_MAX,
       max_i = LONG_MIN;
  double min_f = LONG_MAX,
         max_f = LONG_MIN;

  int n;
  Bool scan_error;
  Bool use_stdin;

  Bool show_extrema;
  ipUInt4_t flags;

#ifndef FILENAME_MAX
#define FILENAME_MAX 256
#endif

  char picfile_name[FILENAME_MAX];

  ipPicDescriptor *pic;


  /*--------------- commandline scaning starts here -----------*/
  use_stdin = TRUE;
  scan_error = FALSE;

  show_extrema = FALSE;
  flags = _ipPicInfoNORMAL;

  n = 1;
  while( n < argc )
    {
      if( *argv[n]  == '-' )
	switch( *(argv[n]+1) )
	  {
	    case 'e':
              show_extrema = TRUE;
              break;
	    case 's':
              flags |= _ipPicInfoSHORT;
              break;
	    case 'h':
	    case 'H':
              scan_error = TRUE;
              break;
	    default: fprintf( stderr,  "%s: sorry, unknown option -%c\n", argv[0], *(argv[n]+1) );
		     scan_error = TRUE;
		     break;
	  }
      else
	{
	  if( use_stdin )
	    {
	      use_stdin = FALSE;
	      strcpy( picfile_name, argv[n] );
	    }
	  else
	    scan_error = TRUE;
	}
      n++;
    }

  if( scan_error )
    {
      fprintf( stderr, "Usage:   %s -e -s [picfile]\n", argv[0] );
      fprintf( stderr, "  -e         calculate the extrema\n" );
      fprintf( stderr, "  -s         short output\n" );
      fprintf( stderr, "  picfile    the picfile to examine, if not given stdin is used\n" );
      exit( ipError );
    }

  /*--------------- commandline scaning ends here -------------*/

  /* make filenames */
  if( !use_stdin)
    if( (strrchr(picfile_name, '/') >= strrchr(picfile_name, '.')) )
      strcat( picfile_name, ".pic" );

  /* read picfile */
  if( use_stdin )
    pic = ipPicGetHeader( "stdin",
                          NULL );
  else
    {
      pic = ipPicGetHeader( picfile_name,
                            NULL );
      pic = ipPicGetTags( picfile_name,
                          pic );
    }

  if( pic == NULL )
    {
      fprintf( stderr, "sorry, can't read %s\n", picfile_name );
      exit( ipError );
    }

  printf( "\n" );
  printf( "%s\n",picfile_name );

  printf(  "%.*s\n", _ipPicTAGLEN, pic->info->version );
  printf(  "--------------------\n" );

  printf(  "type: %s [%i]\n", ipPicTypeName(pic->type), pic->type );

  printf(  "bpe : %i\n", pic->bpe );

  printf(  "dim : %i ", pic->dim );
  for( i=0; i<pic->dim; i++ )
    printf(  "[%i] ", pic->n[i] );
  printf(  "\n" );

  if( pic != NULL
      && pic->info->tags_head != NULL )
    printf(  "--------------------\n" );

  _ipPicInfoPrintTags( stdout,
                       pic->info->tags_head,
                       1,
                       flags );

  printf(  "--------------------\n" );
  printf(  "size of the image data: %u\n", _ipPicSize( pic ) );


  if( show_extrema )
    {
      pic = ipPicGet( picfile_name,
                      pic );

      switch( ipPicDR( pic->type, pic->bpe ) )
        {
          case ipPicDR( ipPicUInt, 8 ):
            {
              MIN_MAX( min_i, max_i, ipUInt1_t );
              printf( "min: %li, max: %li\n", min_i, max_i );
            }
            break;
          case ipPicDR( ipPicInt, 16 ):
            {
              MIN_MAX( min_i, max_i, ipInt2_t );
              printf( "min: %li, max: %li\n", min_i, max_i );
            }
            break;
          case ipPicDR( ipPicInt, 32 ):
            {
              MIN_MAX( min_i, max_i, ipInt4_t );
              printf( "min: %li, max: %li\n", min_i, max_i );
            }
            break;
          case ipPicDR( ipPicFloat, 32 ):
            {
              MIN_MAX( min_f, max_f, ipFloat4_t );
              printf( "min: %f, max: %f\n", min_f, max_f );
            }
            break;
          case ipPicDR( ipPicFloat, 64 ):
            {
              MIN_MAX( min_f, max_f, ipFloat8_t );
              printf( "min: %f, max: %f\n", min_f, max_f );
            }
            break;
          default :
            printf( "sorry, datatype not supported. (min,max)\n" );
            break;
        }
    }

  ipPicFree( pic );

  return( 0 );
}
