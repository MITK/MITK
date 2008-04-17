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
 *  this functions calculates the square root of all pixels
 */

/** @brief calculates the square root of all pixels
 *
 *  @param pic_1      pointer to the first image
 *  @param keep       tells whether the image type could be changed to ipPicFloat
 *                  keep =  : image data type could be changed
 *                  keep =  : image data type of original pictures is
 *                             kept (if there will be an over-/underflow
 *                             the max. or min. possible greyvalue is taken)
 *  @param pic_return  memory which could be used for pic_new. If
 *                pic_retrun == NULL new memory is allocated
 *
 * @return pointer to the new image
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncSqrt ( ipPicDescriptor *pic_1,
                              ipFuncFlagI_t    keep,
                              ipPicDescriptor *pic_return );
#ifndef DOXYGEN_IGNORE

/* definition of macros                                                 */

#define SQRT1( type_1, pic_1, pic_new )                                  \
{                                                                        \
  mitkIpPicFORALL_2 ( SQRT2, pic_new, pic_1, type_1 );                       \
}                                                                        \
 

#define SQRT2( type_n, pic_new, pic_1, type_1 )                          \
{                                                                        \
  ipUInt4_t  i, no_elem;                                                 \
  ipFloat8_t help;                                                       \
                                                                         \
  no_elem = _ipPicElements ( pic_1 );                                    \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
       help  = ( ipFloat8_t ) (( type_1 * ) pic_1->data ) [i];           \
       (( type_n * ) pic_new->data ) [i] =                               \
          ( help < 0. ) ? 0 : ( type_n ) sqrt ( help );                  \
    }                                                                    \
}



/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncSqrt ( ipPicDescriptor *pic_1,
                              ipFuncFlagI_t    keep,
                              ipPicDescriptor *pic_return )
{

  ipPicDescriptor *pic_new;         /* pointer to new image             */
  ipUInt4_t       i;                /* loop index                       */
  ipFloat8_t      min1, max1;       /* extreme greyvalues of 1. image   */ 


  /* check image data                                                   */

  if ( _ipFuncError ( pic_1 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. greyvalues of both images                  */

  if ( ipFuncExtr ( pic_1, &min1, &max1 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues for data type of images*/

  /* find out data type of new iamge                                    */

  if ( keep == ipFuncKeep )
    {
       pic_new = _ipFuncMalloc ( pic_1, pic_return, mitkIpOVERWRITE );
       if ( pic_new == NULL ) return ( mitkIpFuncERROR );
    }
  else if ( keep == ipFuncNoKeep )
    {
       pic_new = ipPicNew ();
       pic_new->dim  = pic_1->dim;
       pic_new->bpe  = 64;
       pic_new->type = ipPicFloat;
       for ( i = 0; i < pic_new->dim; i++ ) 
          pic_new->n[i] = pic_1->n[i];
    }
  else 
    {
       _ipFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new == NULL )
    {  
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( keep == ipFuncNoKeep )
    pic_new->data = malloc ( _ipPicSize  ( pic_new ) );
  if ( pic_new->data == NULL )
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* macro to invert the picture (for all data types)                   */

  mitkIpPicFORALL_1 (  SQRT1, pic_1, pic_new )

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_1);
                        
  return pic_new;
}
#endif
