/** @file
 *  calculates the number of elements in the TSV's value
 */

#ifndef lint
  static char *what = { "@(#)_ipPicTSVElements\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

 /**  calculates the number of elements in the TSV's value
 * @param tsv	the tsv
 *
 * @return the number of elements in the tsv
 *
 *
 * AUTHOR & DATE
 *  Andre Schroeter	23.01.95
 *
 * UPDATES
 *  none
 */



ipUInt4_t _ipPicTSVElements( ipPicTSV_t *tsv )
{
  ipUInt4_t i;
  ipUInt4_t elements;

  if( tsv->dim == 0 )
    return( 0 );

  elements = tsv->n[0];
  for( i = 1; i < tsv->dim; i++ )
    elements *= tsv->n[i];

  return( elements );
}
