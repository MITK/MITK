
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   copys a slice from a pic into a new pic
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:58  andre
 * Initial revision
 *
 * Revision 0.0  1993/06/07  19:19:19  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicCopySlice\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

ipPicDescriptor *_ipPicCopySlice( ipPicDescriptor *pic, ipPicDescriptor *pic_in, ipUInt4_t slice )
{
  ipUInt4_t picsize;

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );


  if( slice < 1
      || slice > pic_in->n[pic_in->dim - 1] )
    {
      return( pic );
    }

  pic->type = pic_in->type;
  pic->bpe = pic_in->bpe;
  pic->dim = 2;
  pic->n[0] = pic_in->n[0];
  pic->n[1] = pic_in->n[1];

  picsize = _ipPicSize( pic );
  pic->data = malloc( picsize );

  memcpy( pic->data,
	  &((char *)pic_in->data)[(slice-1) * picsize],
	  picsize );

  return( pic );
}
