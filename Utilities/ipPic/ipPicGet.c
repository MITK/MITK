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
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a PicFile from disk
 *
 * $Log$
 * Revision 1.16  2004/01/16 22:11:59  andre
 * big endian bug fix
 *
 * Revision 1.15  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.13  2002/02/27 09:02:56  andre
 * zlib changes
 *
 * Revision 1.12  2002/02/27 08:54:43  andre
 * zlib changes
 *
 * Revision 1.11  2000/05/04 12:52:37  ivo
 * inserted BSD style license
 *
 * Revision 1.10  2000/02/16 14:29:20  andre
 * *** empty log message ***
 *
 * Revision 1.9  1999/11/28  18:22:42  andre
 * *** empty log message ***
 *
 * Revision 1.8  1999/11/28  16:29:43  andre
 * *** empty log message ***
 *
 * Revision 1.7  1999/11/27  20:03:48  andre
 * *** empty log message ***
 *
 * Revision 1.6  1999/11/27  19:29:43  andre
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
  static char *what = { "@(#)mitkIpPicGet\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

mitkIpPicFile_t
_mitkIpPicOpenPicFileIn( const char *path )
{
  mitkIpPicFile_t infile;

  if( path == NULL )
    infile = stdin;
  else if( strcmp(path, "stdin") == 0 )
    infile = stdin;
  else
    infile = mitkIpPicFOpen( path, "rb" );

  if( !infile )
    {
      char buff[1024];

      sprintf( buff, "%s.gz", path ); 

      infile = mitkIpPicFOpen( buff, "rb" ); 
    }

  return( infile );
}

mitkIpPicDescriptor *
mitkIpPicGet( const char *infile_name, mitkIpPicDescriptor *pic )
{
  mitkIpPicFile_t infile;

  mitkIpPicTag_t tag_name;
  mitkIpUInt4_t len;

  mitkIpUInt4_t to_read;

  infile = _mitkIpPicOpenPicFileIn( infile_name );

  if( !infile )
    {
      /*ipPrintErr( "mitkIpPicGet: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  mitkIpPicFRead( tag_name, 1, 4, infile );

  if( strncmp( "\037\213", tag_name, 2 ) == 0 )
    {
      fprintf( stderr, "mitkIpPicGetHeader: sorry, can't read compressed file\n" );
      return( NULL );
    }
  else if( strncmp( mitkIpPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _mitkIpPicOldGet( infile,
                            NULL );
      else
        _mitkIpPicOldGet( infile,
                      pic );
      if( infile != stdin )
        mitkIpPicFClose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = mitkIpPicNew();

  mitkIpPicClear( pic );

  mitkIpPicFRead( &(tag_name[4]), 1, sizeof(mitkIpPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _mitkIpPicTAGLEN );

  mitkIpPicFReadLE( &len, sizeof(mitkIpUInt4_t), 1, infile );

  mitkIpPicFReadLE( &(pic->type), sizeof(mitkIpUInt4_t), 1, infile );
  mitkIpPicFReadLE( &(pic->bpe), sizeof(mitkIpUInt4_t), 1, infile );
  mitkIpPicFReadLE( &(pic->dim), sizeof(mitkIpUInt4_t), 1, infile );

  mitkIpPicFReadLE( &(pic->n), sizeof(mitkIpUInt4_t), pic->dim, infile );


  to_read = len -        3 * sizeof(mitkIpUInt4_t)
                - pic->dim * sizeof(mitkIpUInt4_t);
#if 0
  mitkIpPicFSeek( infile, to_read, SEEK_CUR );
#else
  pic->info->tags_head = _mitkIpPicReadTags( pic->info->tags_head, to_read, infile, mitkIpPicEncryptionType(pic) );
#endif

  pic->info->write_protect = mitkIpFalse;

#ifdef WIN
  if ((pic->hdata = GlobalAlloc( GMEM_MOVEABLE, _mitkIpPicSize(pic) )) != 0)
    pic->data = GlobalLock( pic->hdata );
#else
  pic->data = malloc( _mitkIpPicSize(pic) );
#endif

  pic->info->pixel_start_in_file = mitkIpPicFTell( infile );
  if( pic->type == mitkIpPicNonUniform )
    mitkIpPicFRead( pic->data, pic->bpe / 8, _mitkIpPicElements(pic), infile );
  else
    mitkIpPicFReadLE( pic->data, pic->bpe / 8, _mitkIpPicElements(pic), infile );

  if( infile != stdin )
    mitkIpPicFClose( infile );

#ifdef WIN
  GlobalUnlock( pic->hdata );
#endif

  return( pic );
}

_mitkIpPicTagsElement_t *
_mitkIpPicReadTags( _mitkIpPicTagsElement_t *head, mitkIpUInt4_t bytes_to_read, FILE *stream, char encryption_type )
{
  while( bytes_to_read > 0 )
    {
      mitkIpPicTSV_t *tsv;

      mitkIpPicTag_t tag_name;
      mitkIpUInt4_t len;

      /*printf( "bytes_to_read: %i\n", bytes_to_read ); */

      tsv = malloc( sizeof(mitkIpPicTSV_t) );

      mitkIpPicFRead( &tag_name, 1, sizeof(mitkIpPicTag_t), stream );
      strncpy( tsv->tag, tag_name, _mitkIpPicTAGLEN );
      tsv->tag[_mitkIpPicTAGLEN] = '\0';

      mitkIpPicFReadLE( &len, sizeof(mitkIpUInt4_t), 1, stream );

      mitkIpPicFReadLE( &(tsv->type), sizeof(mitkIpUInt4_t), 1, stream );
      mitkIpPicFReadLE( &(tsv->bpe), sizeof(mitkIpUInt4_t), 1, stream );
      mitkIpPicFReadLE( &(tsv->dim), sizeof(mitkIpUInt4_t), 1, stream );


      mitkIpPicFReadLE( &(tsv->n), sizeof(mitkIpUInt4_t), tsv->dim, stream );

      /*printf( "%.*s\n", _mitkIpPicTAGLEN, tsv->tag );
      printf( "  %i\n", len );
      printf( "  %i %i %i %i %i\n",
              tsv->type,
              tsv->bpe,
              tsv->dim,
              tsv->n[0],
              tsv->n[1] ); */

      if( tsv->type == mitkIpPicTSV )
        {
          tsv->value = NULL;
          tsv->value = _mitkIpPicReadTags( tsv->value,
                                       len -        3 * sizeof(mitkIpUInt4_t)
                                           - tsv->dim * sizeof(mitkIpUInt4_t),
                                       stream,
                                       encryption_type );
        }
      else
        {
          mitkIpUInt4_t  elements;

          elements = _mitkIpPicTSVElements( tsv );

          if( tsv->type == mitkIpPicASCII
              || tsv->type == mitkIpPicNonUniform )
            tsv->bpe = 8;

assert( elements * tsv->bpe / 8 == len
                                   -        3 * sizeof(mitkIpUInt4_t)
                                   - tsv->dim * sizeof(mitkIpUInt4_t) );

          if( tsv->type == mitkIpPicASCII )
            tsv->value = malloc( elements+1 * tsv->bpe / 8 );
          else
            tsv->value = malloc( elements * tsv->bpe / 8 );

          mitkIpPicFReadLE( tsv->value, tsv->bpe / 8, elements, stream );

          if( tsv->type == mitkIpPicASCII )
            ((char *)(tsv->value))[elements] = '\0';

          if( encryption_type == 'e'
              && ( tsv->type == mitkIpPicASCII
                   || tsv->type == mitkIpPicNonUniform ) )
            {
              if( tsv->type == mitkIpPicNonUniform )
                {
                  sprintf( tsv->tag, "*** ENCRYPTED ***" );
                  tsv->tag[_mitkIpPicTAGLEN] = '\0';
                }

              free( tsv->value );

              tsv->value = strdup( "*** ENCRYPTED ***" );
              tsv->n[0] = strlen(tsv->value);
              tsv->type = mitkIpPicASCII;
              tsv->dim = 1;
            }
        }

      head = _mitkIpPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(mitkIpUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
