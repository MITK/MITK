/*
 * ipPicTSVSize.c
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  calculates the size of the TSV's value in bytes
 *
 * FUNCTION DECLARATION
 *  ipUInt4_t _ipPicTSVSize( ipPicTSV_t *tsv )
 *
 * PARAMETERS
 *  tsv			the tsv
 *
 * RETURN VALUES
 *  the size of the tsv in bytes
 *
 *
 * AUTHOR & DATE
 *  Andre Schroeter	23.01.95
 *
 * UPDATES
 *  none
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)_ipPicTSVSize\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

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
