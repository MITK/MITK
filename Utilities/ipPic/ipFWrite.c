
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   writes big- and litle- endian to disk
 *   and swaps to the other endianess
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:58  andre
 * Initial revision
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
  static char *what = { "@(#)_ipFWrite\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ipPic.h"

size_t _ipFWrite( void *ptr, size_t size, size_t nitems, FILE *stream )
{
  size_t bytes_return;

  void *buff;
  buff = malloc( size * nitems );
  _ipCpCvtEndian( ptr, buff, size*nitems, size );
  bytes_return = fwrite( buff, size, nitems, stream);
  free( buff );

  return( bytes_return );
}
