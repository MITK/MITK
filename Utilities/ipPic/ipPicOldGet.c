
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads a oldstyle PicFile from disk
 *
 * $Log$
 * Revision 1.3  2000/01/13 14:03:07  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:17  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
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
  static char *what = { "@(#)_ipPicOldGet\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

#ifdef DOS
#include "ipPicOP.h"
#else
#include "ipPicOldP.h"
#endif

ipPicDescriptor * _ipPicOldGet( FILE *infile, ipPicDescriptor *pic )
{
  _ipPicOldHeader old_pic;
#ifdef WIN
  HANDLE hbuff;
#endif

  /*unsigned char *text;*/
  void *buff;

  unsigned long int elements;

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
      /*text = (unsigned char *)malloc( old_pic.ltxt );
      ipFReadLE( text, 1, old_pic.ltxt, infile );*/
      fseek( infile, old_pic.ltxt, SEEK_CUR );
    }


  elements = old_pic.n1 * old_pic.n2;
  if( old_pic.rank == 3 )
    elements *= old_pic.n3;

#ifdef WIN
  if ((hbuff = GlobalAlloc( GMEM_MOVEABLE, elements * old_pic.type )) != 0)
    buff = GlobalLock( hbuff );
#else
  buff = malloc( elements * old_pic.type );
#endif

  ipFReadLE( buff, old_pic.type, elements, infile );

  /* convert to the new pic3 format */

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  pic->data = (void *)buff;
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

#ifdef WIN
  GlobalUnlock( hbuff );
  pic->hdata = hbuff;
#endif

  return( pic );
}
