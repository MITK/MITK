
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a slice from a PicFile
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:59  andre
 * Initial revision
 *
 * Revision 0.1  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.0  1993/03/31  11:26:16  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicGetSlice\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicGetSlice( char *infile_name, ipPicDescriptor *pic, ipUInt4_t slice )
{
  FILE *infile;

  ipPicTag_t tag_name;
  ipUInt4_t len;
  ipUInt4_t skip;

  unsigned long int picsize;

  if( infile_name == NULL )
    infile = stdin;
  else if( strcmp(infile_name, "stdin") == 0 )
    infile = stdin;
  else
    infile = fopen( infile_name, "rb" );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGetSlice: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  fread( &(tag_name[0]), 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _ipPicOldGetSlice( infile,
                                 NULL,
                                 slice );
      else
        _ipPicOldGetSlice( infile,
                           pic,
                           slice );
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


  skip = len -        3 * sizeof(ipUInt4_t)
             - pic->dim * sizeof(ipUInt4_t);
  fseek( infile, skip, SEEK_CUR );

  picsize = _ipPicSize(pic);

  pic->dim = 2;

  if( slice < 1
      || slice > picsize / _ipPicSize(pic) )
    {
      ipPicClear( pic );
      return( pic );
    }

  pic->info->write_protect = ipTrue;

  picsize = _ipPicSize(pic);

  fseek( infile, picsize * (slice - 1), SEEK_CUR );

  pic->data = malloc( picsize );

  ipFReadLE( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );

  if( infile != stdin )
    fclose( infile );

  return( pic );
}
