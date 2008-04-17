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
 *  this function performs a smoothing by a mean-value filter
 */

/** @brief performs a smoothing by a mean-value filter     
 *
 *  @param pic_old    pointer to original image
 *  @param len_mask  number of pixels for each dimension
 *  @param dim_mask   number of dimensions of filter in mask
 *  @param border   handling of the edge
 *   @arg @c     ipFuncBorderOld  : original greyvalues
 *   @arg @c     ipFuncBorderZero : edge pixels are set to zero
 *
 *  @return pointer to the smoothed image
 *
 * @par Uses @arg ipFuncConv() convolve image with mask
 *
 * AUTHOR & DATE
 */

/* include files                                                         */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncMeanF   ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       len_mask,
                                 ipUInt4_t       dim_mask,  
                                 ipFuncFlagI_t   border );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif




/* --------------------------------------------------------------------- */
/* 
*/
/* --------------------------------------------------------------------- */
 
ipPicDescriptor *ipFuncMeanF   ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       len_mask,
                                 ipUInt4_t       dim_mask,  
                                 ipFuncFlagI_t   border )
{
  ipPicDescriptor *pic_new;          /* pointer to new image structure   */
  ipPicDescriptor *pic_mask;         /* pointer to mask                  */
  ipUInt4_t       i;                 /* loop index                       */
  ipUInt4_t       no_elem;
  ipFloat8_t      help;

  /* check data                                                          */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( pic_old->dim < dim_mask || dim_mask < 1 )
    {
       _ipFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( len_mask % 2 != 1 ) 
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );   
    }

  /* initialize mask                                                     */
  
  pic_mask       = ipPicNew();
  
  if ( pic_mask == NULL ) 
    {
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_mask->type = ipPicFloat;
  pic_mask->bpe  = 64;
  pic_mask->dim  = dim_mask;

  for ( i = 0; i < dim_mask; i++ ) pic_mask->n[i] = len_mask;

  pic_mask->data = malloc ( _ipPicSize ( pic_mask ) );
  if ( pic_mask->data == NULL )
    {
       ipPicFree ( pic_mask->data );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  no_elem        = _ipPicElements ( pic_mask );
  help           = 1. / ( ipFloat8_t ) no_elem;                       
  for ( i = 0; i < no_elem; i++ )                       
    (( ipFloat8_t * ) pic_mask->data ) [i] = help;

  /* convolve image with filtering mask                                 */

  pic_new = ipFuncConv ( pic_old, pic_mask, border );

  ipPicFree ( pic_mask );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  

  return ( pic_new );
}

#endif
