
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   frees all space used by a picDescriptor
 *
 * $Log$
 * Revision 1.5  2000/05/04 12:35:59  ivo
 * some doxygen comments.
 *
 * Revision 1.4  1998/09/16  18:37:34  andre
 * added ipPicFreeTag
 *
 * Revision 1.3  1998/09/16  18:32:45  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:12  andre
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
  static char *what = { "@(#)ipPicFree\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
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

void
ipPicFreeTag( ipPicTSV_t *tsv )
{
  if( tsv )
    {  
      if( tsv->value );
        free( tsv->value );

      free( tsv );
    }  
}
