/** @file 
 *  calculates the size of the image data
 */

#ifndef lint
  static char *what = { "@(#)_ipPicSize\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

/** calculates the size of the image data
 *  @param pic pointer to image
 *  @return size of image data
 *
 * $Log$
 * Revision 1.3  2000/05/04 12:36:01  ivo
 * some doxygen comments.
 *
 * Revision 1.2  1997/09/15  13:21:20  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 * Revision 0.0  1993/04/26  15:28:03  andre
 * Initial revision, NO error checking
 */

ipUInt4_t _ipPicSize(
#if defined(__cplusplus) || defined(c_plusplus)
                         const
#endif
                         ipPicDescriptor *pic )
{
  ipUInt4_t i;
  ipUInt4_t elements;

  if( pic->dim == 0 )
    return( 0 );

  elements = pic->n[0];
  for( i = 1; i < pic->dim; i++ )
    elements *= pic->n[i];

  return( elements * pic->bpe / 8 );
}
