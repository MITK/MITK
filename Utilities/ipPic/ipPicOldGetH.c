
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads the header of a oldstyle PicFile from disk
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:59  andre
 * Initial revision
 *
 * Revision 0.2  1993/04/26  17:40:09  andre
 * 8 bit images from old pic format are now unsigned
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
  static char *what = { "@(#)_ipPicOldGetHeader\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

#ifdef DOS
#include "ipPicOP.h"
#else
#include "ipPicOldP.h"
#endif

ipPicDescriptor * _ipPicOldGetHeader( FILE *infile, ipPicDescriptor *pic )
{
  _ipPicOldHeader old_pic;

  /*unsigned char *text;*/

  /* read infile */
  ipFReadLE( &(old_pic.dummy1), sizeof(ipUInt4_t), 4, infile );
  if( old_pic.conv <= 0 || old_pic.conv > 6 )
    {
      old_pic.conv = 3;
      old_pic.rank = 2;
    }

  ipFReadLE( &(old_pic.n1), sizeof(ipUInt4_t),old_pic.rank, infile );
  if( old_pic.rank == 3 && old_pic.n3 == 1 )
    old_pic.rank = 2;

  ipFReadLE( &(old_pic.type), sizeof(ipUInt4_t), 3, infile );
  if( old_pic.ntxt )
    {
      /*text = (unsigned char *)malloc( old_pic.ltxt );
      ipFReadLE( text, 1, old_pic.ltxt, infile );*/
      fseek( infile, old_pic.ltxt, SEEK_CUR );
    }

  /* convert to the new pic3 format */

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  pic->data = NULL;
  if( old_pic.type == 1 )
    pic->type = ipPicUInt;
  else
    pic->type = old_pic.conv;
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
