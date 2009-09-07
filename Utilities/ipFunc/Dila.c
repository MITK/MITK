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
*  this function performs a dilation.  If the original
 *  image is a binary image each pixel is connected with the surrounding
 *  pixels which are non zero in the kernel by using logical operations 
 *  ( and for erosion, or for dilation ). Greylevel images are transformed
 *  by taking the minimum (erosion) of the surrounding pixels or the      
 *  maximum (dilation).
 */

 /** @brief performs a dilation.  If the original
 *  image is a binary image each pixel is connected with the surrounding
 *  pixels which are non zero in the kernel by using logical operations 
 *  ( and for erosion, or for dilation ). Greylevel images are transformed
 *  by taking the minimum (erosion) of the surrounding pixels or the      
 *  maximum (dilation).
 *
 *  @param pic_old pointer to the original image
 *  @param mask pointer to the kernel
 *  @param border tells how the edge is transformed
 *  @arg @c mitkIpFuncBorderOld  :   original greyvalues
 *  @arg @c mitkIpFuncBorderZero :   edge is set to zero
 *
 *  @return pointer to the transformed image
 *
 *
 * AUTHOR & DATE
 */

/* include files */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncDila  ( mitkIpPicDescriptor *pic_old,
                               mitkIpPicDescriptor *pic_mask,
                               mitkIpFuncFlagI_t   border );
#ifndef DOXYGEN_IGNORE

mitkIpPicDescriptor *mitkIpFuncDila  ( mitkIpPicDescriptor *pic_old,
                               mitkIpPicDescriptor *pic_mask,
                               mitkIpFuncFlagI_t   border )
{ 
   mitkIpPicDescriptor *pic_new;

   pic_new = _mitkIpFuncMorph ( pic_old, pic_mask, mitkIpFuncDilaF, border );
   if ( border == mitkIpFuncBorderZero )
     pic_new = mitkIpFuncBorder ( pic_new, pic_mask, pic_new );


   mitkIpFuncCopyTags(pic_new, pic_old);
   
   return ( pic_new );
}

#endif

