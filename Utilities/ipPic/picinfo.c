/*****************************************************************************

 Copyright (c) 1993-2000,  Div. Medical and Biological Informatics, 
 Deutsches Krebsforschungszentrum, Heidelberg, Germany
 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

 - All advertising materials mentioning features or use of this software must 
   display the following acknowledgement: 
          
     "This product includes software developed by the Div. Medical and 
      Biological Informatics, Deutsches Krebsforschungszentrum, Heidelberg, 
      Germany."

 - Neither the name of the Deutsches Krebsforschungszentrum nor the names of 
   its contributors may be used to endorse or promote products derived from 
   this software without specific prior written permission. 

   THIS SOFTWARE IS PROVIDED BY THE DIVISION MEDICAL AND BIOLOGICAL
   INFORMATICS AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE DIVISION MEDICAL AND BIOLOGICAL INFORMATICS,
   THE DEUTSCHES KREBSFORSCHUNGSZENTRUM OR CONTRIBUTORS BE LIABLE FOR 
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

 Send comments and/or bug reports to:
   mbi-software@dkfz-heidelberg.de

*****************************************************************************/


/*
 * $RCSfile$
 *---------------------------------------------------------------
 * DESCRIPTION
 *   prints information about a picfile
 *
 * $Log$
 * Revision 1.7  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.6  2002/10/31 10:00:30  ivo
 * zlib.lib including headers now in win32. gz-extension should work now on every windows.
 * WIN32 define changed to standard _WIN32.
 *
 * Revision 1.5  2002/04/18 19:09:13  ivo
 * compression (zlib) activated.
 *
 * Revision 1.4  2000/05/04 12:52:42  ivo
 * inserted BSD style license
 *
 * Revision 1.3  2000/05/04 12:36:02  ivo
 * some doxygen comments.
 *
 * Revision 1.2  1997/09/15  13:21:22  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:10:00  andre
 * initial import
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
  static char *what = { "@(#)picinfo \t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

/*
** defines
*/

/*
** standard includes
*/
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <strings.h>
#else
#include <string.h>
#endif

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
  flags = _mitkIpPicInfoNORMAL;

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
              flags |= _mitkIpPicInfoSHORT;
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

  printf(  "%.*s\n", _mitkIpPicTAGLEN, pic->info->version );
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

      switch( mitkIpPicDR( pic->type, pic->bpe ) )
        {
          case mitkIpPicDR( ipPicUInt, 8 ):
            {
              MIN_MAX( min_i, max_i, ipUInt1_t );
              printf( "min: %li, max: %li\n", min_i, max_i );
            }
            break;
          case mitkIpPicDR( ipPicInt, 16 ):
            {
              MIN_MAX( min_i, max_i, ipInt2_t );
              printf( "min: %li, max: %li\n", min_i, max_i );
            }
            break;
          case mitkIpPicDR( ipPicInt, 32 ):
            {
              MIN_MAX( min_i, max_i, ipInt4_t );
              printf( "min: %li, max: %li\n", min_i, max_i );
            }
            break;
          case mitkIpPicDR( ipPicFloat, 32 ):
            {
              MIN_MAX( min_f, max_f, ipFloat4_t );
              printf( "min: %f, max: %f\n", min_f, max_f );
            }
            break;
          case mitkIpPicDR( ipPicFloat, 64 ):
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
