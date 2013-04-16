/*****************************************************************************

 Copyright (c) 1993-2000,  Div. Medical and Biological Informatics, 
 Deutsches Krebsforschungszentrum, Heidelberg, Germany
 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

 - All advertising materials mentioning features or use of this software must 
   display the following acknowledgement: 
          
     "This product includes software developed by the Div. Medical and 
      Biological Informatics, Deutsches Krebsforschungszentrum, Heidelberg, 
      Germany."

 - Neither the name of the Deutsches Krebsforschungszentrum nor the names of 
   its contributors may be used to endorse or promote products derived from 
   this software without specific prior written permission. 

   THIS SOFTWARE IS PROVIDED BY THE DIVISION MEDICAL AND BIOLOGICAL
   INFORMATICS AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE DIVISION MEDICAL AND BIOLOGICAL INFORMATICS,
   THE DEUTSCHES KREBSFORSCHUNGSZENTRUM OR CONTRIBUTORS BE LIABLE FOR 
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

 Send comments and/or bug reports to:
   mbi-software@dkfz-heidelberg.de

*****************************************************************************/


/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   Allocates memory for a new PicDescriptor
 *
 * $Log$
 * Revision 1.7  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.5  2000/05/04 12:52:39  ivo
 * inserted BSD style license
 *
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
  static char *what = { "@(#)mitkIpPicNew\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

mitkIpPicDescriptor *mitkIpPicNew( void )
{
  mitkIpPicDescriptor *pic;

  pic = malloc( sizeof(mitkIpPicDescriptor) );

  if( pic != NULL )
    {
      pic->data = NULL;
      pic->type = mitkIpPicUnknown;

      pic->info = malloc( sizeof(_mitkIpPicInfo_t) );

      strncpy( pic->info->version, mitkIpPicVERSION, _mitkIpPicTAGLEN );

      pic->info->tags_head = NULL;

      pic->info->write_protect = mitkIpFalse;
      pic->info->pixel_start_in_file = 0;
      pic->info->lender = 0;
      pic->info->borrower = 0;
      pic->info->ref = 1;
    }

  return( pic );
}
