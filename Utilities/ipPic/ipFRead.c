
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   reads big- and litle- endian from disk
 *   and swaps to the other endianess
 *
 * $Log$
 * Revision 1.3  1999/11/27 19:15:08  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:08  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:58  andre
 * initial import
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
  static char *what = { "@(#)_ipFRead\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ipPic.h"

size_t
_ipFRead( void *ptr, size_t size, size_t nitems, FILE *stream )
{
  size_t bytes_return;

  bytes_return = ipFRead( ptr, size, nitems, stream);
  _ipCvtEndian( ptr, size*nitems, size );

  return( bytes_return );
}
