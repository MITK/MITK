
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a tag from a PicFile
 *
 * $Log$
 * Revision 1.6  1999/11/28 00:36:09  andre
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/27  19:32:13  andre
 * *** empty log message ***
 *
 * Revision 1.2.2.2  1998/03/25  15:03:36  andre
 * added info->pixel_start_in_file
 *
 * Revision 1.2.2.1  1997/09/15  13:47:06  andre
 * added encryption
 *
 * Revision 1.2  1997/09/15  13:21:14  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicGetTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicGetTags( char *infile_name, ipPicDescriptor *pic )
{
  ipFile_t infile;

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
  ipFRead( &(tag_name[0]), 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( infile != stdin )
        ipFClose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = ipPicNew();

  ipFRead( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  /*strncpy( pic->info->version, tag_name, _ipPicTAGLEN );*/

  ipFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &dim, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( n, sizeof(ipUInt4_t), dim, infile );


  to_read = len -        3 * sizeof(ipUInt4_t)
                -      dim * sizeof(ipUInt4_t);

  pic->info->tags_head = _ipPicReadTags( pic->info->tags_head, to_read, infile, ipPicEncryptionType(pic) );

  pic->info->pixel_start_in_file = ftell( infile );

  if( infile != stdin )
    ipFClose( infile );

  return( pic );
}
