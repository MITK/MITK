/*
 * ipPicInfo.c
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  prints information about an ipPicDescriptor
 *
 * FUNCTION DECLARATION
 *  void _ipPicInfo( FILE *stream, ipPicDescriptor *pic, ipUInt4_t flags )
 *
 * PARAMETERS
 *  stream	the output FILE*
 *  pic		the pic	
 *  flags	which information to show	
 *
 * RETURN VALUES
 *  none
 *
 * AUTHOR & DATE
 *  Andre Schroeter	03.10.94
 *
 * UPDATES
 *   none
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)_ipPicInfo\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

#define INT_MAX 5
#define ASCII_MAX 60

/*
** private macros
*/
#define Min(x, y) (((x) < (y)) ? (x) : (y))
#define Max(x, y) (((x) > (y)) ? (x) : (y))

void _ipPicInfo( FILE *stream, ipPicDescriptor *pic, ipUInt4_t flags )
{
  ipUInt4_t i;

  fprintf( stream, "%.*s\n", _ipPicTAGLEN, pic->info->version );
  fprintf( stream, "--------------------\n" );

  fprintf( stream, "type: %s [%i]\n", ipPicTypeName(pic->type), pic->type );

  fprintf( stream, "bpe : %i\n", pic->bpe );

  fprintf( stream, "dim : %i ", pic->dim );
  for( i=0; i<pic->dim; i++ )
    fprintf( stream, "[%i] ", pic->n[i] );
  fprintf( stream, "\n" );

  if( pic != NULL
      && pic->info->tags_head != NULL )
    fprintf( stream, "--------------------\n" );

  _ipPicInfoPrintTags( stream,
                       pic->info->tags_head,
                       1,
                       flags );

  fprintf( stream, "--------------------\n" );
  fprintf( stream, "size of the image data: %u\n", _ipPicSize( pic ) );
}

void _ipPicInfoPrintTags( FILE *stream, _ipPicTagsElement_t* head, ipUInt4_t level, ipUInt4_t flags )
{
  ipUInt4_t i;
  _ipPicTagsElement_t *current;

  char *indent;

  indent = malloc( level*4 + 1 );
  for( i = 0; i < level*4-2; i++ )
    indent[i] = ' ';
  indent[i] = '\0';

  current = head;
  while( current != NULL )
    {
      fprintf( stream, "%s%.*s\n", indent, _ipPicTAGLEN, current->tsv->tag );
      if( !(flags & _ipPicInfoSHORT) )
        {
          fprintf( stream, "%s  type: %s [%i]\n", indent,
                           ipPicTypeName(current->tsv->type), current->tsv->type );
          fprintf( stream, "%s  bpe : %i\n", indent, current->tsv->bpe );
          fprintf( stream, "%s  dim : %i ", indent, current->tsv->dim );

          for( i=0; i<current->tsv->dim; i++ )
            fprintf( stream, "[%i] ", current->tsv->n[i] );
          fprintf( stream, "\n" );
        }

      if( current->tsv->type == ipPicTSV )
        {
          _ipPicInfoPrintTags( stream,
                               current->tsv->value,
                               ++level,
                               flags );
        }
      else
        {
          fprintf( stream, "%s  value : ", indent );

          switch( current->tsv->type )
            {
              case ipPicASCII:
                if( current->tsv->n[0] > ASCII_MAX )
                  fprintf( stream, "\"%.*s\"...\n",
                                   ASCII_MAX,
                                   (char *)current->tsv->value );
                else
                  fprintf( stream, "\"%.*s\"\n",
                                   (int)current->tsv->n[0],
                                   (char *)current->tsv->value );
                break;
              case ipPicUInt:
                for( i = 0; i < Min( INT_MAX, current->tsv->n[0] ); i++ )
                  switch( current->tsv->bpe )
                    {
                      case 8:
                        fprintf( stream, "%3u  ",
                                         ((ipUInt1_t *)current->tsv->value)[i] );
                        break;
                      case 16:
                        fprintf( stream, "%5u  ",
                                         ((ipUInt2_t *)current->tsv->value)[i] );
                        break;
                      case 32:
                        fprintf( stream, "%11u  ",
                                         ((ipUInt4_t *)current->tsv->value)[i] );
                        break;
                      default:
                        fprintf( stream, "???" );
                        break;
                    }
                if( current->tsv->n[0] > INT_MAX )
                  fprintf( stream, "..." );
                fprintf( stream, "\n" );
                if( current->tsv->dim > 1 )
                  {
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                  }
                break;
              case ipPicInt:
                for( i = 0; i < Min( INT_MAX, current->tsv->n[0] ); i++ )
                  switch( current->tsv->bpe )
                    {
                      case 8:
                        fprintf( stream, "%3i  ",
                                         ((ipInt1_t *)current->tsv->value)[i] );
                        break;
                      case 16:
                        fprintf( stream, "%5i  ",
                                         ((ipInt2_t *)current->tsv->value)[i] );
                        break;
                      case 32:
                        fprintf( stream, "%11i  ",
                                         ((ipInt4_t *)current->tsv->value)[i] );
                        break;
                      default:
                        fprintf( stream, "???" );
                        break;
                    }
                if( current->tsv->n[0] > INT_MAX )
                  fprintf( stream, "..." );
                fprintf( stream, "\n" );
                if( current->tsv->dim > 1 )
                  {
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                    fprintf( stream, "%s          .\n", indent );
                  }
                break;
              case ipPicFloat:
                for( i = 0; i < Min( INT_MAX, current->tsv->n[0] ); i++ )
                  switch( current->tsv->bpe )
                    {
                      case 32:
                        fprintf( stream, "%5.5f  ",
                                         ((ipFloat4_t *)current->tsv->value)[i] );
                        break;
                      case 64:
                        fprintf( stream, "%5.5f  ",
                                         ((ipFloat8_t *)current->tsv->value)[i] );
                        break;
                      default:
                        fprintf( stream, "???" );
                        break;
                    }
                if( current->tsv->n[0] > INT_MAX )
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
