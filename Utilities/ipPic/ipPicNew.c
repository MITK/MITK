
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   Allocates memory for a new PicDescriptor
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:59  andre
 * Initial revision
 *
 * Revision 0.0  1993/04/06  09:54:58  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicNew\t\tDKFZ (Dept. MBI)\t$Date$" };
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
    }

  return( pic );
}
