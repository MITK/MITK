/*============================================================================

 Copyright (c) German Cancer Research Center (DKFZ)
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

     "This product includes software developed by the German Cancer Research
      Center (DKFZ)."

 - Neither the name of the German Cancer Research Center (DKFZ) nor the names
   of its contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE GERMAN CANCER RESEARCH CENTER (DKFZ) AND
   CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
   BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE GERMAN
   CANCER RESEARCH CENTER (DKFZ) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
   DAMAGE.

============================================================================*/


/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   frees all space used by a picDescriptor
 *
 * $Log$
 * Revision 1.8  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.6  2000/05/04 12:52:37  ivo
 * inserted BSD style license
 *
 * Revision 1.5  2000/05/04 12:35:59  ivo
 * some doxygen comments.
 *
 * Revision 1.4  1998/09/16  18:37:34  andre
 * added mitkIpPicFreeTag
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
 *
 */
#ifndef lint
  static char *what = { "@(#)mitkIpPicFree\t\tGerman Cancer Research Center (DKFZ)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

void mitkIpPicFree( mitkIpPicDescriptor *pic )
{
  if( pic != NULL )
    {
      mitkIpPicClear( pic );

      if( pic->info != NULL )
        {
          free( pic->info );
        }

      free( pic );

      pic = NULL;
   }
}

void
mitkIpPicFreeTag( mitkIpPicTSV_t *tsv )
{
  if( tsv )
    {
      if( tsv->value );
        free( tsv->value );

      free( tsv );
    }
}
