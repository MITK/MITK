
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a tag from a PicFile
 *
 * $Log$
 * Revision 1.3  1997/10/20 13:35:39  andre
 * *** empty log message ***
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
  FILE *infile;

  ipPicTag_t tag_name;
  ipUInt4_t dummy;
  ipUInt4_t len;
  ipUInt4_t dim;
  ipUInt4_t n[_ipPicNDIM];

  ipUInt4_t to_read;

  if( infile_name == NULL )
    infile = stdin;
  else if( strcmp(infile_name, "stdin") == 0 )
    infile = stdin;
  else
    infile = fopen( infile_name, "rb" );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGetTags: sorry, error opening infile\n" );*/
      return( NULL );
    }

  if( pic != NULL )
    pic->info->write_protect = ipFalse;

  /* read infile */
  fread( &(tag_name[0]), 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( infile != stdin )
        fclose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = ipPicNew();

  fread( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  /*strncpy( pic->info->version, tag_name, _ipPicTAGLEN );*/

  ipFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &dim, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( n, sizeof(ipUInt4_t), dim, infile );


  to_read = len -        3 * sizeof(ipUInt4_t)
                -      dim * sizeof(ipUInt4_t);

  pic->info->tags_head = _ipPicReadTags( pic->info->tags_head, to_read, infile, ipPicEncryptionType(pic)  );

  if( infile != stdin )
    fclose( infile );

  return( pic );
}
