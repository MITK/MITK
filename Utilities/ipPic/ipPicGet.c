
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a PicFile from disk
 *
 * $Log$
 * Revision 1.6  1999/11/27 19:29:43  andre
 * *** empty log message ***
 *
 * Revision 1.5  1998/09/04  17:45:54  andre
 * *** empty log message ***
 *
 * Revision 1.4  1998/05/06  14:13:15  andre
 * added info->pixel_start_in_file
 *
 * Revision 1.3  1997/10/20  13:35:39  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:13  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
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
  static char *what = { "@(#)ipPicGet\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipFile_t
_ipPicOpenPicFileIn( char *path )
{
  ipFile_t infile;
  if( path == NULL )
    infile = stdin;
  else if( strcmp(path, "stdin") == 0 )
    infile = stdin;
  else
    infile = ipFOpen( path, "rb" );
  return( infile );
}

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
  pic->info->tags_head = _ipPicReadTags( pic->info->tags_head, to_read, infile, ipPicEncryptionType(pic) );
#endif

  pic->info->write_protect = ipFalse;

#ifdef WIN
  if ((pic->hdata = GlobalAlloc( GMEM_MOVEABLE, _ipPicSize(pic) )) != 0)
    pic->data = GlobalLock( pic->hdata );
#else
  pic->data = malloc( _ipPicSize(pic) );
#endif

  pic->info->pixel_start_in_file = ftell( infile );
  if( pic->type == ipPicNonUniform )
    fread( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );
  else
    ipFReadLE( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );

  if( infile != stdin )
    fclose( infile );

#ifdef WIN
  GlobalUnlock( pic->hdata );
#endif

  return( pic );
}

_ipPicTagsElement_t *_ipPicReadTags( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, FILE *stream, char encryption_type )
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
                                       stream,
                                       encryption_type );
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

          if( encryption_type == 'e'
              && ( tsv->type == ipPicASCII
                   || tsv->type == ipPicNonUniform ) )
            {
              if( tsv->type == ipPicNonUniform )
                {
                  sprintf( tsv->tag, "*** ENCRYPTED ***" );
                  tsv->tag[_ipPicTAGLEN] = '\0';
                }

              free( tsv->value );

              tsv->value = strdup( "*** ENCRYPTED ***" );
              tsv->n[0] = strlen(tsv->value);
              tsv->type = ipPicASCII;
              tsv->dim = 1;
            }
        }

      head = _ipPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(ipUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
