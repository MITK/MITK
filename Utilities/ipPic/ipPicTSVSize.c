/** @file
 *  calculates the size of the TSV's value in bytes
 */

#ifndef lint
  static char *what = { "@(#)_ipPicTSVSize\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

 /**  calculates the size of the TSV's value in bytes
 *  @param tsv	the tsv
 *
 *  @return the size of the tsv in bytes
 *
 *
 * AUTHOR & DATE
 *  Andre Schroeter	23.01.95
 *
 * UPDATES
 *  none
 */


ipUInt4_t _ipPicTSVSize( ipPicTSV_t *tsv )
{
  ipUInt4_t i;
  ipUInt4_t elements;

  if( tsv->dim == 0 )
    return( 0 );

  elements = tsv->n[0];
  for( i = 1; i < tsv->dim; i++ )
    elements *= tsv->n[i];

  return( elements * tsv->bpe / 8 );
}
