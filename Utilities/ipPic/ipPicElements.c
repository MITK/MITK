
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   calculates number of Elements in the data
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)_ipPicElements\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

ipUInt4_t _ipPicElements( ipPicDescriptor *pic )
{
  int i;
  ipUInt4_t elements;

  if( pic->dim == 0 )
    return( 0 );

  elements = pic->n[0];
  for( i = 1; i < pic->dim; i++ )
    elements *= pic->n[i];

  return( elements );
}
