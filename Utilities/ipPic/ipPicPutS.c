
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   writes a PicFile to disk
 *
 * $Log$
 * Revision 1.7  2000/05/04 12:36:01  ivo
 * some doxygen comments.
 *
 * Revision 1.6  2000/01/17  18:32:00  andre
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/27  19:15:08  andre
 * *** empty log message ***
 *
 * Revision 1.4  1998/09/01  15:26:43  andre
 * *** empty log message ***
 *
 * Revision 1.3  1998/05/18  12:13:54  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:19  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 * Revision 0.0  1993/04/02  16:18:39  andre
 * Initial revision
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicPutSlice\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

void ipPicPutSlice( char *outfile_name, ipPicDescriptor *pic, ipUInt4_t slice )
{
  ipPicDescriptor *pic_in;

  ipUInt4_t tags_len;

  FILE *outfile;

  pic_in = ipPicGetHeader( outfile_name,
                           NULL );

  if( pic_in == NULL )
    {
      if( slice == 1 )
        {
          pic->n[pic->dim] = 1;
          pic->dim += 1;

          ipPicPut( outfile_name, pic );

          pic->dim -= 1;
          pic->n[pic->dim] = 0;

          return;
        }
      else
        return;
    }

  pic_in = ipPicGetTags( outfile_name,
                         pic_in );

  outfile = fopen( outfile_name, "r+b" );

  if( outfile == NULL )
    {
      /*ipPrintErr( "ipPicPut: sorry, error opening outfile\n" );*/
      /*return();*/
    }

  if( pic->dim != pic_in->dim - 1 )
    {
      fclose( outfile );
      return;
    }
  else if( pic->n[0] != pic_in->n[0]  )
    {
      fclose( outfile );
      return;
    }
  else if( pic->n[1] != pic_in->n[1]  )
    {
      fclose( outfile );
      return;
    }

  if( slice > pic_in->n[pic_in->dim-1] )
    pic_in->n[pic_in->dim-1] += 1;

  /* write oufile */
  /*fseek( outfile, 0, SEEK_SET );*/
  rewind( outfile );
  fwrite( ipPicVERSION, 1, sizeof(ipPicTag_t), outfile );

  fseek( outfile, sizeof(ipUInt4_t), SEEK_CUR ); /* skip tags_len */

  ipFWriteLE( &(pic_in->type), sizeof(ipUInt4_t), 1, outfile );
  ipFWriteLE( &(pic_in->bpe), sizeof(ipUInt4_t), 1, outfile );
  ipFWriteLE( &(pic_in->dim), sizeof(ipUInt4_t), 1, outfile );

  ipFWriteLE( pic_in->n, sizeof(ipUInt4_t), pic_in->dim, outfile );

  fseek( outfile, pic_in->info->pixel_start_in_file + _ipPicSize(pic) * (slice - 1), SEEK_SET );

  ipFWriteLE( pic->data, pic->bpe / 8, _ipPicElements(pic), outfile );

  /*fseek( outfile, 0, SEEK_END );*/

  fclose( outfile );

  ipPicFree(pic_in);

  /*return();*/
}
