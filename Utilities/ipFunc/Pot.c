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
 * This function takes all greyvalues of the image to the power of the exponent.
 */

/** @brief takes all greyvalues of the image to the power of the exponent. 
 *
 *  @param pic_1     pointer to the first image
 *  @param exponent    exponent                             
 *  @param keep       tells whether the image type could be changed when
 *                necessary
 *  @arg @c     ipFuncNoKeep : image data type could be changed
 *  @arg @c     ipFuncKeep   : image data type of original pictures is
 *                                 kept (if there will be an over-/underflow
 *                                 the max. or min. possible greyvalue is taken)
 *  @param pic_return  memory which could be used for pic_new. If
 *                pic_retrun == NULL new memory is allocated
 *
 * @return pointer to the new image
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncPot ( ipPicDescriptor *pic_1,
                             ipFloat8_t      exponent,
                             ipFuncFlagI_t   keep,
                             ipPicDescriptor *pic_return );
#ifndef DOXYGEN_IGNORE

/* definition of macros                                                 */

#define POT1( type_1, pic_1, exponent, pic_new )                         \
{                                                                        \
  mitkIpPicFORALL_3 ( POT2, pic_new, pic_1, exponent, type_1 );              \
}                                                                        \
 

#define POT2( type_n, pic_new, pic_1, exponent, type_1 )                 \
{                                                                        \
  ipUInt4_t  i, no_elem;                                                 \
  ipFloat8_t help;                                                       \
                                                                         \
  no_elem = _ipPicElements ( pic_1 );                                    \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
       help  = pow( ( ipFloat8_t) (( type_1 * ) pic_1->data ) [i],       \
                    exponent );                                          \
       (( type_n * ) pic_new->data ) [i] =                               \
          ( max_gv >  help  ) ?                                          \
             (( min_gv <  help ) ?  (type_n)help  : ( type_n )min_gv ) : \
             ( type_n ) max_gv;                                          \
    }                                                                    \
}



/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncPot ( ipPicDescriptor *pic_1,
                             ipFloat8_t      exponent,
                             ipFuncFlagI_t   keep,
                             ipPicDescriptor *pic_return )
{

  ipPicDescriptor *pic_new;         /* pointer to new image             */
  ipFloat8_t      max_gv;           /* max. possible greyvalue          */
  ipFloat8_t      min_gv;           /* min. possible greyvalue          */
  ipFloat8_t      min1, max1;       /* extreme greyvalues of 1. image   */ 
  ipFloat8_t      smin, smax;       /* product of extreme greyvalues    */


  /* check image data                                                   */

  if ( _ipFuncError ( pic_1 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. greyvalues of both images                  */

  if ( ipFuncExtr ( pic_1, &min1, &max1 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues for data type of images*/

  if ( _ipFuncExtT ( pic_1->type, pic_1->bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
    return ( mitkIpFuncERROR );

  /* find out data type of new iamge                                    */

  if ( keep == ipFuncKeep )
    {
       pic_new = _ipFuncMalloc ( pic_1, pic_return, mitkIpOVERWRITE );
       if ( pic_new == NULL ) return ( mitkIpFuncERROR );
    }
  else if ( keep == ipFuncNoKeep )
    {
       smax      = pow ( max1, exponent );
       smin      = pow ( min1, exponent );

       /* change image type of images of type ipPicInt                  */

       if ( pic_1->type == ipPicInt )
         {
           if ( smax < max_gv && smin > min_gv )
             {
                pic_new = ipPicCopyHeader ( pic_1, NULL );
             }
           else
             {
                pic_new       = ipPicCopyHeader ( pic_1, NULL );
                pic_new->type = ipPicFloat;
                pic_new->bpe  = 64;
                _ipFuncExtT ( pic_new->type, pic_new->bpe, &min_gv, &max_gv );
             }
         }

       /* change image type of images of type ipPicUInt                 */

       else if ( pic_1->type == ipPicUInt )
         {
           if ( smax < max_gv && smin > min_gv  )
             {
                pic_new = ipPicCopyHeader ( pic_1, NULL );
             }
           else
             {
                pic_new = ipPicCopyHeader ( pic_1, NULL );
                pic_new->type = ipPicInt;
                pic_new->bpe  = 16;
                _ipFuncExtT ( pic_new->type, pic_new->bpe, &min_gv, &max_gv );
                if ( smax > max_gv || smin < min_gv )
                  {
                     pic_new->type = ipPicFloat;
                     pic_new->bpe  = 64;
                     _ipFuncExtT ( pic_new->type, pic_new->bpe, &min_gv, &max_gv );
                  }

             }
         } 

       /* change image type of images of type ipPicUInt                 */
 
       else if ( pic_1->type == ipPicFloat )
         {
            pic_new = ipPicCopyHeader ( pic_1, NULL );
         }
       else 
         { 
            _ipFuncSetErrno ( mitkIpFuncTYPE_ERROR );
            return ( mitkIpFuncERROR );
         }
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

  mitkIpPicFORALL_2 (  POT1, pic_1, exponent, pic_new )

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_1);
                        
  return pic_new;
}
#endif

