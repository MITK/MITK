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
 *   writes a PicFile to disk
 *
 * $Log$
 * Revision 1.17  2005/09/09 09:14:45  ivo
 * FIX: warnings on windows
 *
 * Revision 1.16  2005/06/21 11:22:48  ivo
 * CHG: encrypted only a warning instead of an error on write
 *
 * Revision 1.15  2003/02/18 12:28:23  andre
 * write compressed pic files
 *
 * Revision 1.14  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.12  2002/02/27 09:06:28  andre
 * zlib changes
 *
 * Revision 1.11  2000/05/04 12:52:40  ivo
 * inserted BSD style license
 *
 * Revision 1.10  2000/05/04 12:36:01  ivo
 * some doxygen comments.
 *
 * Revision 1.9  1999/11/27  19:40:44  andre
 * *** empty log message ***
 *
 * Revision 1.8  1999/11/27  19:24:19  andre
 * *** empty log message ***
 *
 * Revision 1.3.2.3  1998/03/25  15:03:36  andre
 * added info->pixel_start_in_file
 *
 * Revision 1.3.2.2  1997/10/09  11:03:53  andre
 * *** empty log message ***
 *
 * Revision 1.3.2.1  1997/09/15  13:46:15  andre
 * added encryption
 *
 * Revision 1.3  1997/09/15  13:21:18  andre
 * switched to new what string format
 *
 * Revision 1.2  1997/09/15  10:24:13  andre
 * ipPicPut now returns status
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
  static char *what = { "@(#)ipPicPut\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

extern size_t
_ipPicFWrite( const void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream);

int
ipPicPut( const char *outfile_name, ipPicDescriptor *pic )
{
  FILE *outfile;

  ipUInt4_t len;
  ipUInt4_t tags_len;

  if( pic->info->write_protect )
    {
      fprintf( stderr, "ipPicPut: sorry, can't write (missing tags !!!)\n" );
      return( -1 );
    }

  if( mitkIpPicEncryptionType(pic) != ' ' )
    {
      fprintf( stderr, "ipPicPut: warning: was encrypted !!!\n" );
    }

  if( outfile_name == NULL )
    outfile = stdout;
  else if( strcmp(outfile_name, "stdout") == 0 )
    outfile = stdout;
  else
    {
      ipPicRemoveFile( outfile_name );

      if( ipPicGetWriteCompression() )
        {
          char buff[1024];

          sprintf( buff, "%s.gz", outfile_name );
          outfile = mitkIpPicFOpen( buff, "wb" );
        }
      else
        outfile = fopen( outfile_name, "wb" );
    }


  if( outfile == NULL )
    {
      fprintf( stderr, "ipPicPut: sorry, error opening outfile\n" );
      return( -1 );
    }

  tags_len = _ipPicTagsSize( pic->info->tags_head );

  len = tags_len +        3 * sizeof(ipUInt4_t)
                 + pic->dim * sizeof(ipUInt4_t);

  /* write oufile */
  if( mitkIpPicEncryptionType(pic) == ' ' )
    mitkIpPicFWrite( mitkIpPicVERSION, 1, sizeof(ipPicTag_t), outfile );
  else
    mitkIpPicFWrite( pic->info->version, 1, sizeof(ipPicTag_t), outfile );

  mitkIpPicFWriteLE( &len, sizeof(ipUInt4_t), 1, outfile );

  mitkIpPicFWriteLE( &(pic->type), sizeof(ipUInt4_t), 1, outfile );
  mitkIpPicFWriteLE( &(pic->bpe), sizeof(ipUInt4_t), 1, outfile );
  mitkIpPicFWriteLE( &(pic->dim), sizeof(ipUInt4_t), 1, outfile );

  mitkIpPicFWriteLE( pic->n, sizeof(ipUInt4_t), pic->dim, outfile );

  _ipPicWriteTags( pic->info->tags_head, outfile, mitkIpPicEncryptionType(pic) );

  if( ipPicGetWriteCompression() )
    pic->info->pixel_start_in_file = mitkIpPicFTell( outfile );
  else
    pic->info->pixel_start_in_file = ftell( outfile );

  if( pic->data )
    {
      if( pic->type == ipPicNonUniform )
        mitkIpPicFWrite( pic->data, pic->bpe / 8, _ipPicElements(pic), outfile );
      else
        mitkIpPicFWriteLE( pic->data, pic->bpe / 8, _ipPicElements(pic), outfile );
    }

  if( outfile != stdout )
    {
      if( ipPicGetWriteCompression() )
        mitkIpPicFClose( outfile );
      else
        fclose( outfile );
    }

  return( 0 );
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

void _ipPicWriteTags( _ipPicTagsElement_t *head, FILE *stream, char encryption_type )
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

      mitkIpPicFWrite( current->tsv->tag, 1, sizeof(ipPicTag_t), stream );

      mitkIpPicFWriteLE( &len, sizeof(ipUInt4_t), 1, stream );

      mitkIpPicFWriteLE( &(current->tsv->type), sizeof(ipUInt4_t), 1, stream );
      mitkIpPicFWriteLE( &(current->tsv->bpe), sizeof(ipUInt4_t), 1, stream );
      mitkIpPicFWriteLE( &(current->tsv->dim), sizeof(ipUInt4_t), 1, stream );
      mitkIpPicFWriteLE( &(current->tsv->n), sizeof(ipUInt4_t), current->tsv->dim, stream );

      if( current->tsv->type == ipPicTSV )
        {
          _ipPicWriteTags( current->tsv->value,
                           stream,
                           encryption_type );
        }
      else
        {
          mitkIpPicFWriteLE( current->tsv->value, current->tsv->bpe / 8, elements, stream );
        }

      current = current->next;
    }
}
