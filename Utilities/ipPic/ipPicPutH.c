
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   writes a PicFile Header to disk
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicPutHeader\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

void ipPicPutHeader( char *outfile_name, ipPicDescriptor *pic )
{
  FILE *outfile;

  ipUInt4_t len;

  if( outfile_name == NULL )
    outfile = stdout;
  else if( strcmp(outfile_name, "stdout") == 0 )
    outfile = stdout;
  else
    outfile = fopen( outfile_name, "wb" );

  if( outfile == NULL )
    {
      /*ipPrintErr( "ipPicPut: sorry, error opening outfile\n" );*/
      return();
    }

  len =          3 * sizeof(ipUInt4_t)
        + pic->dim * sizeof(ipUInt4_t);

  /* write oufile */
  fwrite( ipPicVERSION, 1, sizeof(ipPicTag_t), outfile );

  ipFWriteLE( &len, sizeof(ipUInt4_t), 1, outfile );

  ipFWriteLE( &(pic->type), sizeof(ipUInt4_t), 1, outfile );
  ipFWriteLE( &(pic->bpe), sizeof(ipUInt4_t), 1, outfile );
  ipFWriteLE( &(pic->dim), sizeof(ipUInt4_t), 1, outfile );

  ipFWriteLE( pic->n, sizeof(ipUInt4_t), pic->dim, outfile );

  if( outfile != stdout )
    fclose( outfile );

  /*return();*/
}
