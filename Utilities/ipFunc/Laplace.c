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
 *  This function performs a filtering with a 2D or 3D Laplace filtering  
 *  mask
 */

/** This function performs a filtering with a 2D or 3D Laplace filtering  
 *  mask
 *
 *  @param pic_old      pointer to the image that should be convolved 
 *  @param dim_mask   dimension of filtering mask                        
 *  @param border     tells how the edge is transformed              
 *  @arg @c     ipFuncBorderOld  : original greyvalues         
 *   @arg @c    ipFuncBorderZero : edge is set to minimal greyvalue
 *
 *  @return  pointer to the transformed image   
 *
 * USES
 *  function ipFuncConv: convolves image with mask
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"   

ipPicDescriptor *ipFuncLaplace( ipPicDescriptor *pic_old,           
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncLaplace\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


 
ipPicDescriptor *ipFuncLaplace( ipPicDescriptor *pic_old,           
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border )
{

  ipPicDescriptor *pic_new;           /* convolved  image               */
  ipPicDescriptor *pic_mask;          /* laplace mask                   */
  ipUInt4_t       i;                  /* loop index                     */
  ipInt2_t        laplace2[] =        /* 2D laplace-mask                */
                  {  0, -1,  0, -1,  4, -1,  0, -1,  0 };
  ipInt2_t        laplace3[] =        /* 3D laplace-mask                */
                  {  0,  0,  0,  0, -1,  0,  0,  0,  0,
                     0, -1,  0, -1,  6, -1,  0, -1,  0, 
                     0,  0,  0,  0, -1,  0,  0,  0,  0 };

  /* check whether data are correct                                     */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( pic_old->dim < dim_mask || dim_mask < 2  || dim_mask > 3 ) 
    {
       _ipFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of pic_mask                                         */

  pic_mask = ipPicNew ();
  if ( pic_mask == NULL ) 
    {
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_mask->type = ipPicInt;
  pic_mask->bpe  = 16;
  pic_mask->dim  = dim_mask;  
  for ( i = 0; i < dim_mask; i++ )
    pic_mask->n[i] = 3;

  if ( dim_mask == 3 ) 
    { 
       pic_mask->data = laplace3;   
    }
  else if ( dim_mask == 2 )
    {
       pic_mask->data = laplace2;
    }
  else 
    {
       pic_mask->data = NULL;
       ipPicFree ( pic_mask );
       _ipFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* function to convolve  image with mask                              */

  pic_new = ipFuncConv ( pic_old, pic_mask, border );

  pic_mask->data = NULL;
  ipPicFree ( pic_mask );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  

  return pic_new;
}
#endif

