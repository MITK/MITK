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
 * Revision 1.4  2005/10/13 11:21:12  ivo
 * FIX: linker warning (unused var)
 *
 * Revision 1.3  2005/10/13 08:35:47  ivo
 * FIX: warnings
 *
 * Revision 1.2  2005/10/13 07:52:55  max
 * fixed warnings  Algorithms/mitkImageFilters/mitkImageToSurfaceFilter.h  Algorithms/mitkImageFilters/mitkManualSegmentationToSurfaceFilter.h  Algorithms/mitkImageIO/mitkIpPicGet.c
 *
 * Revision 1.1  2005/10/12 19:08:51  ivo
 * ADD: substitute for ipPicGet: original always deletes data pointer and re-allocs it using malloc, which is not compatible with mitk::ImageDataItem.
 *
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

#include "DataStructures/ipPic/ipPic.h"
#ifdef DOS
#include "ipPicOP.h"
#else
#include "ipPicOldP.h"
#endif

char * MITKstrdup (const char * string)
{
  char *memory;

  if (string==NULL)
    return NULL;

  if ( ( memory = (char*)malloc(strlen(string) + 1) ) )
    return strcpy(memory,string);

  return NULL;
}

_ipPicTagsElement_t *
_MITKipPicReadTags( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, FILE *stream, char encryption_type );

ipPicDescriptor * _MITKipPicOldGet( FILE *infile, ipPicDescriptor *pic )
{
  _ipPicOldHeader old_pic;

  unsigned long int elements;

  ipUInt4_t size;
  size = 0;
  if( pic == NULL )
    pic = ipPicNew();
  else
  {
    size= _ipPicSize(pic);
    if(pic->data == NULL)
      size = 0;
  }

  /* read infile */
  ipFReadLE( &(old_pic.dummy1), sizeof(ipUInt4_t), 4, infile );
  if( old_pic.conv <= 0 || old_pic.conv > 6 )
    {
      old_pic.conv = 3;
      old_pic.rank = 2;
    }

  ipFReadLE( &(old_pic.n1), sizeof(ipUInt4_t), old_pic.rank, infile );
  if( old_pic.rank == 3 && old_pic.n3 == 1 )
    old_pic.rank = 2;

  ipFReadLE( &(old_pic.type), sizeof(ipUInt4_t), 3, infile );
  if( old_pic.ntxt )
    {
      fseek( infile, old_pic.ltxt, SEEK_CUR );
    }


  elements = old_pic.n1 * old_pic.n2;
  if( old_pic.rank == 3 )
    elements *= old_pic.n3;

  if((size == 0) || (size != _ipPicSize(pic)))
    {
      if( pic->data != NULL )
        {
          free( pic->data );
          pic->data = NULL;
        }
      pic->data = malloc( _ipPicSize(pic) );
    }

  ipFReadLE( pic->data, old_pic.type, elements, infile );

  /* convert to the new pic3 format */

  if( old_pic.type == 1 )
    pic->type = ipPicUInt;
  else
    pic->type = (ipPicType_t)old_pic.conv;
  pic->bpe = old_pic.type*8;
  pic->dim = old_pic.rank;
  pic->n[0] = old_pic.n1;
  pic->n[1] = old_pic.n2;
  pic->n[2] = old_pic.n3;
  pic->n[3] = old_pic.n4;
  pic->n[4] = old_pic.n5;
  pic->n[5] = old_pic.n6;
  pic->n[6] = old_pic.n7;
  pic->n[7] = old_pic.n8;

  return( pic );
}

ipPicDescriptor *
MITKipPicGet( char *infile_name, ipPicDescriptor *pic )
{
  ipPicFile_t infile;

  ipPicTag_t tag_name;
  ipUInt4_t len;

  ipUInt4_t to_read;
  ipUInt4_t size;

  infile = _ipPicOpenPicFileIn( infile_name );

  if( !infile )
    {
      /*ipPrintErr( "ipPicGet: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  ipPicFRead( tag_name, 1, 4, infile );

  if( strncmp( "\037\213", tag_name, 2 ) == 0 )
    {
      fprintf( stderr, "ipPicGetHeader: sorry, can't read compressed file\n" );
      return( NULL );
    }
  else if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _MITKipPicOldGet( infile,
                            NULL );
      else
        _MITKipPicOldGet( infile,
                      pic );
      if( infile != stdin )
        ipPicFClose( infile );
      return( pic );
    }

  size = 0;
  if( pic == NULL )
    pic = ipPicNew();
  else
  {
    size= _ipPicSize(pic);
    if(pic->data == NULL)
      size = 0;
  }

  if( pic->info != NULL )
  {
    _ipPicFreeTags( pic->info->tags_head );
    pic->info->tags_head = NULL;
  }

  ipPicFRead( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _ipPicTAGLEN );

  ipPicFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipPicFReadLE( &(pic->type), sizeof(ipUInt4_t), 1, infile );
  ipPicFReadLE( &(pic->bpe), sizeof(ipUInt4_t), 1, infile );
  ipPicFReadLE( &(pic->dim), sizeof(ipUInt4_t), 1, infile );

  ipPicFReadLE( &(pic->n), sizeof(ipUInt4_t), pic->dim, infile );


  to_read = len -        3 * sizeof(ipUInt4_t)
                - pic->dim * sizeof(ipUInt4_t);
#if 0
  ipPicFSeek( infile, to_read, SEEK_CUR );
#else
  pic->info->tags_head = _MITKipPicReadTags( pic->info->tags_head, to_read, infile, ipPicEncryptionType(pic) );
#endif

  pic->info->write_protect = ipFalse;

  if((size == 0) || (size != _ipPicSize(pic)))
    {
      if( pic->data != NULL )
        {
          free( pic->data );
          pic->data = NULL;
        }
#ifdef WIN
      if ((pic->hdata = GlobalAlloc( GMEM_MOVEABLE, _ipPicSize(pic) )) != 0)
        pic->data = GlobalLock( pic->hdata );
#else
      pic->data = malloc( _ipPicSize(pic) );
#endif
    }

  pic->info->pixel_start_in_file = ipPicFTell( infile );
  if( pic->type == ipPicNonUniform )
    ipPicFRead( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );
  else
    ipPicFReadLE( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );

  if( infile != stdin )
    ipPicFClose( infile );

#ifdef WIN
  GlobalUnlock( pic->hdata );
#endif

  return( pic );
}

_ipPicTagsElement_t *
_MITKipPicReadTags( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, FILE *stream, char encryption_type )
{
  while( bytes_to_read > 0 )
    {
      ipPicTSV_t *tsv;

      ipPicTag_t tag_name;
      ipUInt4_t len;

      /*printf( "bytes_to_read: %i\n", bytes_to_read ); */

      tsv = malloc( sizeof(ipPicTSV_t) );

      ipPicFRead( &tag_name, 1, sizeof(ipPicTag_t), stream );
      strncpy( tsv->tag, tag_name, _ipPicTAGLEN );
      tsv->tag[_ipPicTAGLEN] = '\0';

      ipPicFReadLE( &len, sizeof(ipUInt4_t), 1, stream );

      ipPicFReadLE( &(tsv->type), sizeof(ipUInt4_t), 1, stream );
      ipPicFReadLE( &(tsv->bpe), sizeof(ipUInt4_t), 1, stream );
      ipPicFReadLE( &(tsv->dim), sizeof(ipUInt4_t), 1, stream );


      ipPicFReadLE( &(tsv->n), sizeof(ipUInt4_t), tsv->dim, stream );

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

          ipPicFReadLE( tsv->value, tsv->bpe / 8, elements, stream );

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

              tsv->value = MITKstrdup( "*** ENCRYPTED ***" );
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

ipPicDescriptor *
MITKipPicGetTags( char *infile_name, ipPicDescriptor *pic )
{
  ipPicFile_t infile;

  ipPicTag_t tag_name;
  ipUInt4_t dummy;
  ipUInt4_t len;
  ipUInt4_t dim;
  ipUInt4_t n[_ipPicNDIM];

  ipUInt4_t to_read;

  infile = _ipPicOpenPicFileIn( infile_name );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGetTags: sorry, error opening infile\n" );*/
      return( NULL );
    }

  if( pic != NULL )
    pic->info->write_protect = ipFalse;

  /* read infile */
  ipPicFRead( &(tag_name[0]), 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( infile != stdin )
        ipPicFClose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = ipPicNew();

  ipPicFRead( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  /*strncpy( pic->info->version, tag_name, _ipPicTAGLEN );*/

  ipPicFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipPicFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  ipPicFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  ipPicFReadLE( &dim, sizeof(ipUInt4_t), 1, infile );

  ipPicFReadLE( n, sizeof(ipUInt4_t), dim, infile );


  to_read = len -        3 * sizeof(ipUInt4_t)
                -      dim * sizeof(ipUInt4_t);

  pic->info->tags_head = _MITKipPicReadTags( pic->info->tags_head, to_read, infile, ipPicEncryptionType(pic) );

  pic->info->pixel_start_in_file = ipPicFTell( infile );

  if( infile != stdin )
    ipPicFClose( infile );

  return( pic );
}
