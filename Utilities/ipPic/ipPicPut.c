
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   writes a PicFile to disk
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:59  andre
 * Initial revision
 *
 * Revision 0.1  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicPut\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

void ipPicPut( char *outfile_name, ipPicDescriptor *pic )
{
  FILE *outfile;

  ipUInt4_t len;
  ipUInt4_t tags_len;

  if( pic->info->write_protect )
    {
      fprintf( stderr, "ipPicPut: sorry, can't write (missing tags !!!)\n" );
      return;
    }

  if( outfile_name == NULL )
    outfile = stdout;
  else if( strcmp(outfile_name, "stdout") == 0 )
    outfile = stdout;
  else
    outfile = fopen( outfile_name, "wb" );

  if( outfile == NULL )
    {
      /*ipPrintErr( "ipPicPut: sorry, error opening outfile\n" );*/
      /*return();*/
    }

  tags_len = _ipPicTagsSize( pic->info->tags_head );

  len = tags_len +        3 * sizeof(ipUInt4_t)
                 + pic->dim * sizeof(ipUInt4_t);

  /* write oufile */
  fwrite( ipPicVERSION, 1, sizeof(ipPicTag_t), outfile );

  ipFWriteLE( &len, sizeof(ipUInt4_t), 1, outfile );

  ipFWriteLE( &(pic->type), sizeof(ipUInt4_t), 1, outfile );
  ipFWriteLE( &(pic->bpe), sizeof(ipUInt4_t), 1, outfile );
  ipFWriteLE( &(pic->dim), sizeof(ipUInt4_t), 1, outfile );

  ipFWriteLE( pic->n, sizeof(ipUInt4_t), pic->dim, outfile );

  _ipPicWriteTags( pic->info->tags_head, outfile );

  ipFWriteLE( pic->data, pic->bpe / 8, _ipPicElements(pic), outfile );

  if( outfile != stdout )
    fclose( outfile );

  /*return();*/
}

ipUInt4_t _ipPicTagsSize( _ipPicTagsElement_t *head )
{
  _ipPicTagsElement_t *current = head;
  ipUInt4_t tags_len;

  tags_len = 0;
  while( current != NULL )
    {
      ipUInt4_t  len;

      if( current->tsv->type == ipPicTSV )
        {
          if( current->tsv->dim == 0 )
            {
              current->tsv->n[0] = _ipPicTagsNumber(current->tsv->value);

              if( current->tsv->n[0] > 0 )
                current->tsv->dim = 1;
            }

          len = _ipPicTagsSize( (_ipPicTagsElement_t *)current->tsv->value );
        }
      else
        {
          ipUInt4_t  elements;

          elements = _ipPicTSVElements( current->tsv );

          if( current->tsv->type == ipPicASCII
              || current->tsv->type == ipPicNonUniform )
            current->tsv->bpe = 8;

          len = elements * current->tsv->bpe / 8;
        }

      tags_len += 32                                       /* name           */
                  +                     sizeof( ipUInt4_t) /* len            */
                  +                 3 * sizeof( ipUInt4_t) /* type, bpe, dim */
                  + current->tsv->dim * sizeof( ipUInt4_t) /* n[]            */
                  + len;                                   /* data           */

      current = current->next;
    }
  return( tags_len );
}

ipUInt4_t _ipPicTagsNumber( _ipPicTagsElement_t *head )
{
  _ipPicTagsElement_t *current = head;
  ipUInt4_t tags_number;

  tags_number = 0;
  while( current != NULL )
    {
      tags_number++;

      current = current->next;
    }
  return( tags_number );
}

void _ipPicWriteTags( _ipPicTagsElement_t *head, FILE *stream )
{
  _ipPicTagsElement_t *current = head;

  while( current != NULL )
    {
      ipUInt4_t  elements;
      ipUInt4_t  len;

      elements = _ipPicTSVElements( current->tsv );

      if( current->tsv->type == ipPicTSV )
        {
          if( current->tsv->dim == 0 )
            {
              current->tsv->n[0] = _ipPicTagsNumber(current->tsv->value);

              if( current->tsv->n[0] > 0 )
                current->tsv->dim = 1;
            }

          assert( elements == _ipPicTagsNumber(current->tsv->value) );

          len = _ipPicTagsSize( current->tsv->value );
        }
      else
        {
          if( current->tsv->type == ipPicASCII
              || current->tsv->type == ipPicNonUniform )
            current->tsv->bpe = 8;

          len = elements * current->tsv->bpe / 8;

         }

      len +=                   3 * sizeof(ipUInt4_t)  /* type, bpe, dim */
             + current->tsv->dim * sizeof(ipUInt4_t); /* n[]            */

      fwrite( current->tsv->tag, 1, sizeof(ipPicTag_t), stream );

      ipFWriteLE( &len, sizeof(ipUInt4_t), 1, stream );

      ipFWriteLE( &(current->tsv->type), sizeof(ipUInt4_t), 1, stream );
      ipFWriteLE( &(current->tsv->bpe), sizeof(ipUInt4_t), 1, stream );
      ipFWriteLE( &(current->tsv->dim), sizeof(ipUInt4_t), 1, stream );
      ipFWriteLE( &(current->tsv->n), sizeof(ipUInt4_t), current->tsv->dim, stream );

      if( current->tsv->type == ipPicTSV )
        {
          _ipPicWriteTags( current->tsv->value,
                           stream );
        }
      else
        {
          ipFWriteLE( current->tsv->value, current->tsv->bpe / 8, elements, stream );
        }

      current = current->next;
    }
}
