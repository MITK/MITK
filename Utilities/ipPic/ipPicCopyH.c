
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   copys the header from a pic into a new pic
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicCopyHeader\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicCopyHeader( ipPicDescriptor *pic, ipPicDescriptor *pic_new )
{
  ipUInt4_t i;

  if( pic_new == NULL )
    pic_new = ipPicNew();

  ipPicClear( pic_new );


  pic_new->type = pic->type;
  pic_new->bpe = pic->bpe;
  pic_new->dim = pic->dim;
  for( i=0; i<pic_new->dim; i++ )
    pic_new->n[i] = pic->n[i];

  return( pic_new );
}
