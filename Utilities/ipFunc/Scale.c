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
 *    this function scales an image 
 */

/** @brief this function scales an image
 *
 *     @param pic_old     pointer to the image 
 *     @param sc_fact     vector that contains the scaling factors for each dimension
 *     @param sc_kind     parameter for the kind of interpolation
 *     @arg @c                 mitkIpFuncScaleNN:  next neighbour interpolation
 *     @arg @c                 mitkIpFuncScaleBl:  bilinear interpolation
 *  
 * @return pointer to the scaled image
 *
 * AUTHOR AND DATE
 */
 
/* include-files                                              */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncScale ( mitkIpPicDescriptor *pic_old,
                               ipFloat8_t      sc_fact[_mitkIpPicNDIM],
                               mitkIpFuncFlagI_t   sc_kind ) ;
#ifndef DOXYGEN_IGNORE
 
mitkIpPicDescriptor *mitkIpFuncScale ( mitkIpPicDescriptor *pic_old,
                               ipFloat8_t      sc_fact[_mitkIpPicNDIM],
                               mitkIpFuncFlagI_t   sc_kind ) 
{

  ipUInt4_t        i;             /* loop index               */
  mitkIpPicDescriptor  *pic_new;      /* pointer to scaled image  */

  char            is_color=0;


  /* check whether data are correct                                     */

  if(pic_old->bpe==24)
  {
	  pic_old->bpe=8;
	  is_color=1;
  }

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if(is_color)
	  pic_old->bpe=24;
                                        
  /* allocate pic_new                                         */

  pic_new = mitkIpPicCopyHeader ( pic_old, NULL );
  if ( pic_new == NULL )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* copy scaling information to header of pic_new            */

  pic_new->dim = pic_old->dim;
  for ( i = 0; i < pic_old->dim; i++ )
    pic_new->n[i] = 
       ( sc_fact[i] == 0 ) ?
         pic_old->n[i] :
         ( ipUInt4_t ) ( sc_fact[i] * pic_old->n[i] );
  
  /* call scaling routines                                    */

  if ( sc_kind == mitkIpFuncScaleNN )
    pic_new = _mitkIpFuncScNN ( pic_old, pic_new );
  else if ( sc_kind == mitkIpFuncScaleBl ) {
    pic_new = _mitkIpFuncScBL ( pic_old, pic_new ); 
    /*printf("using NN due to error in _mitkIpFuncScaleBL ... ");
      pic_new = _mitkIpFuncScNN ( pic_old, pic_new );*/
  }
  else
    {
      mitkIpPicFree ( pic_new );
      _mitkIpFuncSetErrno ( mitkIpFuncFLAG_ERROR );
      return ( mitkIpFuncERROR );
    }

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_old);
  
  
                        
  return( pic_new );
}
#endif
