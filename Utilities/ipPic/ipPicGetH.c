
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads the Header of a  PicFile from disk
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
  static char *what = { "@(#)ipPicGetHeader\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicGetHeader( char *infile_name, ipPicDescriptor *pic )
{
  FILE *infile;

  ipPicTag_t tag_name;
  ipUInt4_t len;

  if( infile_name == NULL )
    infile = stdin;
  else if( strcmp(infile_name, "stdin") == 0 )
    infile = stdin;
  else
    infile = fopen( infile_name, "rb" );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGetHeader: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  fread( &(tag_name[0]), 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _ipPicOldGetHeader( infile,
                                  NULL );
      else
        _ipPicOldGetHeader( infile,
                            pic );
      if( infile != stdin )
        fclose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  pic->info->write_protect = ipTrue;

  fread( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _ipPicTAGLEN );

  ipFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &(pic->type), sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &(pic->bpe), sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &(pic->dim), sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &(pic->n), sizeof(ipUInt4_t), pic->dim, infile );


  if( infile != stdin )
    fclose( infile );

  return( pic );
}
