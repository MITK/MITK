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
 *  this function performs a high pass filter using 2D or 3D filtering 
 *  masks
 */

/** @brief performs a high pass filter using 2D or 3D filtering 
 *  masks
 *
 *  @param pic_old      pointer to the image that should be convolved 
 *  @param dim_mask     dimension of filtering mask                        
 *  @param border       tells how the edge is transformed              
 *  @arg @c              mitkIpFuncBorderOld  : original greyvalues         
 *  @arg @c              mitkIpFuncBorderZero : edge is set to zero
 *  @param mask_nr      tells which mask should be taken
 *
 * @return  pointer to the tranformed image 
 *
 * USES
 *  function mitkIpFuncConv: convolves image with mask
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   
 
mitkIpPicDescriptor *mitkIpFuncShp ( mitkIpPicDescriptor *pic_old,           
                             mitkIpUInt4_t       dim_mask, 
                             mitkIpFuncFlagI_t   border,
                             mitkIpUInt4_t       mask_nr ) ;
#ifndef DOXYGEN_IGNORE

mitkIpPicDescriptor *mitkIpFuncShp ( mitkIpPicDescriptor *pic_old,           
                             mitkIpUInt4_t       dim_mask, 
                             mitkIpFuncFlagI_t   border,
                             mitkIpUInt4_t       mask_nr ) 
{

  mitkIpPicDescriptor *pic_new;           /* convolved  image               */
  mitkIpPicDescriptor *pic_mask;          /* high pass mask                 */
  mitkIpUInt4_t       i;                  /* loop index                     */
  mitkIpInt2_t        mask2[] =           /* 2D masks                       */
                  {  0, -1,  0, -1,  5, -1,  0, -1,  0,    /* mask 0    */

                     1, -2,  1, -2,  5, -2,  1, -2,  1,    /* mask 1    */

                    -1, -1, -1, -1,  9, -1, -1, -1, -1 };  /* mask 2    */

  mitkIpInt2_t        mask3[] =           /* 3D masks                       */
                  {  0,  0,  0,  0, -1,  0,  0,  0,  0,    /* mask 0    */
                     0, -1,  0, -1,  7, -1,  0, -1,  0, 
                     0,  0,  0,  0, -1,  0,  0,  0,  0,  

                     1,  1,  1,  1, -4,  1,  1,  1,  1,    /* mask 1    */
                     1, -4,  1, -4,  5, -4,  1, -4,  1, 
                     1,  1,  1,  1, -4,  1,  1,  1,  1,  

                    -1, -1, -1, -1, -1, -1, -1, -1, -1,    /* mask 2    */
                    -1, -1, -1, -1, 27, -1, -1, -1, -1, 
                    -1, -1, -1, -1, -1, -1, -1, -1, -1 };

  /* check whether data are correct                                     */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( pic_old->dim < dim_mask || dim_mask < 2 ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( /* mask_nr < 0 || */ mask_nr > 2 ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of pic_mask                                         */

  pic_mask = mitkIpPicNew ();
  if ( pic_mask == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_mask->type = mitkIpPicInt;
  pic_mask->bpe  = 16;
  pic_mask->dim  = dim_mask;  
  for ( i = 0; i < dim_mask; i++ )
    pic_mask->n[i] = 3;

  if ( dim_mask == 3 ) 
    { 
       pic_mask->data = mask3 + mask_nr * 27;
    }
  else if ( dim_mask == 2 )
    {
       pic_mask->data = mask2 + mask_nr * 9;
    }
  else
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask );
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* function to convolve  image with mask                              */

  pic_new = mitkIpFuncConv ( pic_old, pic_mask, border );

  pic_mask->data = NULL;
  mitkIpPicFree ( pic_mask );

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_old);
  
  
                        
  return pic_new;
}
#endif

