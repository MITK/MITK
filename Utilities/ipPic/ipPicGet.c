
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a PicFile from disk
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
  static char *what = { "@(#)ipPicGet\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicGet( char *infile_name, ipPicDescriptor *pic )
{
  FILE *infile;

  ipPicTag_t tag_name;
  ipUInt4_t len;

  ipUInt4_t to_read;

  if( infile_name == NULL )
    infile = stdin;
  else if( strcmp(infile_name, "stdin") == 0 )
    infile = stdin;
  else
    infile = fopen( infile_name, "rb" );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGet: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  fread( tag_name, 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _ipPicOldGet( infile,
                            NULL );
      else
        _ipPicOldGet( infile,
                      pic );
      if( infile != stdin )
        fclose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  fread( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _ipPicTAGLEN );

  ipFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &(pic->type), sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &(pic->bpe), sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &(pic->dim), sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &(pic->n), sizeof(ipUInt4_t), pic->dim, infile );


  to_read = len -        3 * sizeof(ipUInt4_t)
                - pic->dim * sizeof(ipUInt4_t);
#if 0
  fseek( infile, to_read, SEEK_CUR );
#else
  pic->info->tags_head = _ipPicReadTags( pic->info->tags_head, to_read, infile );
#endif

  pic->info->write_protect = ipFalse;

#ifdef WIN
  if ((pic->hdata = GlobalAlloc( GMEM_MOVEABLE, _ipPicSize(pic) )) != 0)
    pic->data = GlobalLock( pic->hdata );
#else
  pic->data = malloc( _ipPicSize(pic) );
#endif

  ipFReadLE( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );

  if( infile != stdin )
    fclose( infile );

#ifdef WIN
  GlobalUnlock( pic->hdata );
#endif

  return( pic );
}

_ipPicTagsElement_t *_ipPicReadTags( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, FILE *stream )
{
  while( bytes_to_read > 0 )
    {
      ipPicTSV_t *tsv;

      ipPicTag_t tag_name;
      ipUInt4_t len;

      /*printf( "bytes_to_read: %i\n", bytes_to_read ); /**/

      tsv = malloc( sizeof(ipPicTSV_t) );

      fread( &tag_name, sizeof(ipPicTag_t), 1, stream );
      strncpy( tsv->tag, tag_name, _ipPicTAGLEN );
      tsv->tag[_ipPicTAGLEN] = '\0';

      ipFReadLE( &len, sizeof(ipUInt4_t), 1, stream );

      ipFReadLE( &(tsv->type), sizeof(ipUInt4_t), 1, stream );
      ipFReadLE( &(tsv->bpe), sizeof(ipUInt4_t), 1, stream );
      ipFReadLE( &(tsv->dim), sizeof(ipUInt4_t), 1, stream );


      ipFReadLE( &(tsv->n), sizeof(ipUInt4_t), tsv->dim, stream );

      /*printf( "%.*s\n", _ipPicTAGLEN, tsv->tag );
      printf( "  %i\n", len );
      printf( "  %i %i %i %i %i\n",
              tsv->type,
              tsv->bpe,
              tsv->dim,
              tsv->n[0],
              tsv->n[1] ); /**/

      if( tsv->type == ipPicTSV )
        {
          tsv->value = NULL;
          tsv->value = _ipPicReadTags( tsv->value,
                                       len -        3 * sizeof(ipUInt4_t)
                                           - tsv->dim * sizeof(ipUInt4_t),
                                       stream );
        }
      else
        {
          ipUInt4_t  elements;

          elements = _ipPicTSVElements( tsv );

          if( tsv->type == ipPicASCII
              || tsv->type == ipPicNonUniform )
            tsv->bpe = 8;

assert( elements * tsv->bpe / 8 == len
                                   -        3 * sizeof(ipUInt4_t)
                                   - tsv->dim * sizeof(ipUInt4_t) );

          if( tsv->type == ipPicASCII )
            tsv->value = malloc( elements+1 * tsv->bpe / 8 );
          else
            tsv->value = malloc( elements * tsv->bpe / 8 );

          ipFReadLE( tsv->value, tsv->bpe / 8, elements, stream );

          if( tsv->type == ipPicASCII )
            ((char *)(tsv->value))[elements] = '\0';
        }

      head = _ipPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(ipUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
