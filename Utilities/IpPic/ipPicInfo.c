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

/** @file
 *  prints information about an mitkIpPicDescriptor
 */

#ifndef lint
  static char *what = { "@(#)_mitkIpPicInfo\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

/** prints information about an mitkIpPicDescriptor
 *
 *  @param stream	the output FILE*
 *  @param pic		the pic	
 *  @param flags	which information to show	
 *
 *  @return none
 *
 * AUTHOR & DATE
 *  Andre Schroeter	03.10.94
 *
 */


#define INT_MAXIMUM 5
#define ASCII_MAX 60

/*
** private macros
*/
#define Min(x, y) (((x) < (y)) ? (x) : (y))
#define Max(x, y) (((x) > (y)) ? (x) : (y))

void _mitkIpPicInfo( FILE *stream, mitkIpPicDescriptor *pic, mitkIpUInt4_t flags )
{
  mitkIpUInt4_t i;

  fprintf( stream, "%.*s\n", _mitkIpPicTAGLEN, pic->info->version );
  fprintf( stream, "--------------------\n" );

  fprintf( stream, "type: %s [%i]\n", mitkIpPicTypeName(pic->type), pic->type );

  fprintf( stream, "bpe : %i\n", pic->bpe );

  fprintf( stream, "dim : %i ", pic->dim );
  for( i=0; i<pic->dim; i++ )
    fprintf( stream, "[%i] ", pic->n[i] );
  fprintf( stream, "\n" );

  if( pic != NULL
      && pic->info->tags_head != NULL )
    fprintf( stream, "--------------------\n" );

  _mitkIpPicInfoPrintTags( stream,
                       pic->info->tags_head,
                       1,
                       flags );

  fprintf( stream, "--------------------\n" );
  fprintf( stream, "size of the image data: %u\n", _mitkIpPicSize( pic ) );
}

void _mitkIpPicInfoPrintTags( FILE *stream, _mitkIpPicTagsElement_t* head, mitkIpUInt4_t level, mitkIpUInt4_t flags )
{
  mitkIpUInt4_t i;
  _mitkIpPicTagsElement_t *current;

  char *indent;

  indent = malloc( level*4 + 1 );
  for( i = 0; i < level*4-2; i++ )
    indent[i] = ' ';
  indent[i] = '\0';

  current = head;
  while( current != NULL )
    {
      fprintf( stream, "%s%.*s\n", indent, _mitkIpPicTAGLEN, current->tsv->tag );
      if( !(flags & _mitkIpPicInfoSHORT) )
        {
          fprintf( stream, "%s  type: %s [%i]\n", indent,
                           mitkIpPicTypeName(current->tsv->type), current->tsv->type );
          fprintf( stream, "%s  bpe : %i\n", indent, current->tsv->bpe );
          fprintf( stream, "%s  dim : %i ", indent, current->tsv->dim );

          for( i=0; i<current->tsv->dim; i++ )
            fprintf( stream, "[%i] ", current->tsv->n[i] );
          fprintf( stream, "\n" );
        }

      if( current->tsv->type == mitkIpPicTSV )
        {
          _mitkIpPicInfoPrintTags( stream,
                               current->tsv->value,
                               level+1,
                               flags );
        }
      else
        {
          fprintf( stream, "%s  value : ", indent );

          switch( current->tsv->type )
            {
              case mitkIpPicASCII:
                if( current->tsv->n[0] > ASCII_MAX )
                  fprintf( stream, "\"%.*s\"...\n",
                                   ASCII_MAX,
                                   (char *)current->tsv->value );
                else
                  fprintf( stream, "\"%.*s\"\n",
                                   (int)current->tsv->n[0],
                                   (char *)current->tsv->value );
                break;
              case mitkIpPicUInt:
                for( i = 0; i < Min( INT_MAXIMUM, current->tsv->n[0] ); i++ )
                  switch( current->tsv->bpe )
                    {
                      case 8:
                        fprintf( stream, "%3u  ",
                                         ((mitkIpUInt1_t *)current->tsv->value)[i] );
                        break;
                      case 16:
                        fprintf( stream, "%5u  ",
                                         ((mitkIpUInt2_t *)current->tsv->value)[i] );
                        break;
                      case 32:
                        fprintf( stream, "%11u  ",
                                         ((mitkIpUInt4_t *)current->tsv->value)[i] );
                        break;
                      default:
                        fprintf( stream, "???" );
                        break;
                    }
                if( current->tsv->n[0] > INT_MAXIMUM )
                  fprintf( stream, "..." );
                fprintf( stream, "\n" );
                if( current->tsv->dim > 1 )
                  {
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                  }
                break;
              case mitkIpPicInt:
                for( i = 0; i < Min( INT_MAXIMUM, current->tsv->n[0] ); i++ )
                  switch( current->tsv->bpe )
                    {
                      case 8:
                        fprintf( stream, "%3i  ",
                                         ((mitkIpInt1_t *)current->tsv->value)[i] );
                        break;
                      case 16:
                        fprintf( stream, "%5i  ",
                                         ((mitkIpInt2_t *)current->tsv->value)[i] );
                        break;
                      case 32:
                        fprintf( stream, "%11i  ",
                                         ((mitkIpInt4_t *)current->tsv->value)[i] );
                        break;
                      default:
                        fprintf( stream, "???" );
                        break;
                    }
                if( current->tsv->n[0] > INT_MAXIMUM )
                  fprintf( stream, "..." );
                fprintf( stream, "\n" );
                if( current->tsv->dim > 1 )
                  {
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                  }
                break;
              case mitkIpPicFloat:
                for( i = 0; i < Min( INT_MAXIMUM, current->tsv->n[0] ); i++ )
                  switch( current->tsv->bpe )
                    {
                      case 32:
                        fprintf( stream, "%5.5f  ",
                                         ((mitkIpFloat4_t *)current->tsv->value)[i] );
                        break;
                      case 64:
                        fprintf( stream, "%5.5f  ",
                                         ((mitkIpFloat8_t *)current->tsv->value)[i] );
                        break;
                      default:
                        fprintf( stream, "???" );
                        break;
                    }
                if( current->tsv->n[0] > INT_MAXIMUM )
                  fprintf( stream, "..." );
                fprintf( stream, "\n" );
                if( current->tsv->dim > 1 )
                  {
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                  }
                break;
              default:
                fprintf( stream, "???\n" );
                break;
            }
        }

      current = current->next;
    }
  free( indent );
}
