
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads the Header of a  PicFile from disk
 *
 * $Log$
 * Revision 1.7  2000/05/04 12:35:59  ivo
 * some doxygen comments.
 *
 * Revision 1.6  1999/11/28  16:27:21  andre
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/28  00:36:09  andre
 * *** empty log message ***
 *
 * Revision 1.4  1999/11/27  20:02:41  andre
 * *** empty log message ***
 *
 * Revision 1.3  1999/11/27  19:15:26  andre
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
  static char *what = { "@(#)ipPicGetHeader\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicDescriptor *
ipPicGetHeader( char *infile_name, ipPicDescriptor *pic )
{
  ipPicFile_t  infile;

  ipPicTag_t tag_name;
  ipUInt4_t len;

  infile = _ipPicOpenPicFileIn( infile_name );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGetHeader: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  ipPicFRead( &(tag_name[0]), 1, 4, infile );

  if( strncmp( "\037\213", tag_name, 2 ) == 0 )
    {
      fprintf( stderr, "ipPicGetHeader: sorry, can't read compressed file\n" );
      return( NULL );
    }
  else if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _ipPicOldGetHeader( infile,
                                  NULL );
      else
        _ipPicOldGetHeader( infile,
                            pic );
      if( infile != stdin )
        ipPicFClose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  pic->info->write_protect = ipTrue;

  ipPicFRead( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _ipPicTAGLEN );

  ipPicFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipPicFReadLE( &(pic->type), sizeof(ipUInt4_t), 1, infile );
  ipPicFReadLE( &(pic->bpe), sizeof(ipUInt4_t), 1, infile );
  ipPicFReadLE( &(pic->dim), sizeof(ipUInt4_t), 1, infile );

  ipPicFReadLE( &(pic->n), sizeof(ipUInt4_t), pic->dim, infile );


  if( infile != stdin )
    ipPicFClose( infile );

  return( pic );
}
