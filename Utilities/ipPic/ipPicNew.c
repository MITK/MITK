
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   Allocates memory for a new PicDescriptor
 *
 * $Log$
 * Revision 1.4  2000/05/04 12:36:00  ivo
 * some doxygen comments.
 *
 * Revision 1.3  1999/11/27  19:15:08  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:16  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 * Revision 0.0  1993/04/06  09:54:58  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicNew\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicNew( void )
{
  ipPicDescriptor *pic;

  pic = malloc( sizeof(ipPicDescriptor) );

  if( pic != NULL )
    {
      pic->data = NULL;
      pic->type = ipPicUnknown;

      pic->info = malloc( sizeof(_ipPicInfo_t) );

      strncpy( pic->info->version, ipPicVERSION, _ipPicTAGLEN );

      pic->info->tags_head = NULL;

      pic->info->write_protect = ipFalse;
      pic->info->pixel_start_in_file = 0;
    }

  return( pic );
}
