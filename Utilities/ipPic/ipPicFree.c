
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   frees all space used by a picDescriptor
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
  static char *what = { "@(#)ipPicFree\t\tDKFZ (Dept. MBI)\t$Date$" };
#endif

#include "ipPic.h"

void ipPicFree( ipPicDescriptor *pic )
{
  if( pic != NULL )
    {
      ipPicClear( pic );

      if( pic->info != NULL )
        {
          free( pic->info );
        }

      free( pic );

      pic = NULL;
   }
}
