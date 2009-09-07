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

/**@file
 *  this functions performs the morphological operations opening and 
 *  closing.
 */

/** @briefs performs the morphological operations opening and 
 *  closing. 
 *
 *  Opening is an erosion followed by a dilation and a 
 *  closing is a dilation followed by an erosion.
 *
 *  @param pic_old     pointer to the original image
 *  @param mask       pointer to the kernel
 *  @param kind       tells whether opening or closing is performed
 *  @arg  @c      0 (mitkIpOPEN)  :   opening
 *  @arg @c       1 (mitkIpCLOSE) :   closing 
 *  @param border   tells how the edge is transformed
 *  @arg @c       mitkIpFuncBorderOld  :   original greyvalues
 *  @arg @c       mitkIpFuncBorderZero :   edge is set to zero
 *
 * @return pointer to the transformed image
 *
 * USES
 *  function _mitkIpFuncError     - checks the image data          
 *  function _mitkIpFuncMorph     - performs morphological operations
 *
 * AUTHOR & DATE
 */

/* include files                                                        */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *_mitkIpFuncOpCl  ( mitkIpPicDescriptor *pic_old,
                               mitkIpPicDescriptor *mask,
                               _mitkIpFuncFlagF_t    kind, 
                               mitkIpFuncFlagI_t   border );
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif




/* ---------------------------------------------------------------------- */
/* 
** function mitkIpFuncOpCl 
*/
/* ---------------------------------------------------------------------- */

mitkIpPicDescriptor *_mitkIpFuncOpCl  ( mitkIpPicDescriptor *pic_old,
                               mitkIpPicDescriptor *mask,
                               _mitkIpFuncFlagF_t    kind, 
                               mitkIpFuncFlagI_t   border )
{
  mitkIpPicDescriptor *pic_new1;       /* pointer to transformed image         */
  mitkIpPicDescriptor *pic_new2;       /* pointer to transformed image         */
  ipUInt4_t       i;               /* loop index                           */

  /* check image data                                                      */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _mitkIpFuncError ( mask ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( mask->dim > pic_old->dim ) 
    { 
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
  for ( i = 0; i < mask->dim; i++ )
    if ( mask->n[i] > pic_old->n[i] ) 
      { 
         _mitkIpFuncSetErrno ( mitkIpFuncSIZE_ERROR );
         return ( mitkIpFuncERROR );
      }

  /* check whether opening or closing operation should be performed       */

  if ( kind == mitkIpFuncOpenF )
    {
       pic_new1 = _mitkIpFuncMorph ( pic_old, mask, mitkIpFuncEroF, border );
       if ( border == mitkIpFuncBorderZero )
         pic_new1 = mitkIpFuncBorder ( pic_new1, mask, pic_new1 );
       pic_new2 = _mitkIpFuncMorph ( pic_new1, mask, mitkIpFuncDilaF, border );
       if ( border == mitkIpFuncBorderZero )
         pic_new2 = mitkIpFuncBorder ( pic_new2, mask, pic_new2 );
    }
  else if ( kind == mitkIpFuncCloseF )
    {
       pic_new1 = _mitkIpFuncMorph ( pic_old, mask, mitkIpFuncDilaF, border );
       if ( border == mitkIpFuncBorderZero )
         pic_new1 = mitkIpFuncBorder ( pic_new1, mask, pic_new1 );
       pic_new2 = _mitkIpFuncMorph ( pic_new1, mask, mitkIpFuncEroF, border );
       if ( border == mitkIpFuncBorderZero )
         pic_new2 = mitkIpFuncBorder ( pic_new2, mask, pic_new2 );
    }
  else 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }
  mitkIpPicFree ( pic_new1 );

  return ( pic_new2 );
}
#endif
