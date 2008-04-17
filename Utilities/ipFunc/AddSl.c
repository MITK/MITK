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

/** @file
*	this function adds all slices of an image
*/

/**this function adds all slices of an image
 * @param pic pointer to the 3D image
 * @param keep tells whether the image type could be changed when
 *                necessary
 * @arg @c ipFuncNoKeep : image data type could be changed
 * @arg @c ipFuncKeep:    image data type of original pictures 
 *                              is  kept (if there will be an 
 *                              over-/underflow the max. or min. 
 *                              possible greyvalue is taken)
 *
 * @RETURN pointer to the new 2D image
 *
 * @AUTHOR Antje Schroeder
 * @DATE 01.08.95
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */

/* include-Files                                                        */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncAddSl( ipPicDescriptor *pic_old,
                              ipFuncFlagI_t   keep );

#ifndef DOXYGEN_IGNORE

/* definition of macros                                                 */

#define ADDSL( type_old, pic_old, pic_new )                              \
{                                                                        \
   mitkIpPicFORALL_2 ( ADDSL2, pic_new, pic_old, type_old );                 \
}

#define ADDSL2( type_new, pic_new, pic_old, type_old )                   \
{                                                                        \
   ipUInt4_t    i, j;          /* loop counter                        */ \
   ipUInt4_t    off_old;       /* offset in 3D image                  */ \
   ipUInt4_t    slice_size;    /* size of one image slice             */ \
                                                                         \
   slice_size = _ipPicElements ( pic_new );                              \
   for ( i = 0; i < pic_old->n[2]; i++ )                                 \
     {                                                                   \
        off_old = i * slice_size;                                        \
        for ( j = 0; j < slice_size; j++ )                               \
          (( type_new *)pic_new->data)[j] =                              \
             (( type_new *)pic_new->data)[j] +                           \
              ( type_new  )(( type_old *)pic_old->data)[j+off_old];      \
     }                                                                   \
}

#define ADDSL3( type, pic_old, pic_new )                                 \
{                                                                        \
   ipUInt4_t    i, j;          /* loop counter                        */ \
   ipUInt4_t    off_old;       /* offset in 3D image                  */ \
   ipUInt4_t    slice_size;    /* size of one image slice             */ \
   type         help, help2;                                             \
                                                                         \
   slice_size = _ipPicElements ( pic_new );                              \
   for ( i = 0; i < pic_old->n[2]; i++ )                                 \
     {                                                                   \
        off_old = i * slice_size;                                        \
        for ( j = 0; j < slice_size; j++ )                               \
          {                                                              \
             help = (( type *)pic_old->data)[j+off_old];                 \
             help2 = (( type *)pic_new->data)[j];                        \
             (( type *)pic_new->data)[j] =                               \
                 ( max_gv > ( ipFloat8_t ) help + help2 ) ?              \
                    (( min_gv < ( ipFloat8_t ) help + help2 ) ?          \
                        ( (type)help + (type)help2 ) : (type) min_gv ) : \
                    (type) max_gv;                                       \
          }                                                              \
     }                                                                   \
}


/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncAddSl( ipPicDescriptor *pic_old,
                              ipFuncFlagI_t   keep ) 
{

  ipPicDescriptor *pic_new;         /* pointer to new image             */
  ipFloat8_t      max_gv;           /* max. possible greyvalue          */
  ipFloat8_t      min_gv;           /* min. possible greyvalue          */
  ipFloat8_t      min1, max1;       /* extreme greyvalues of 1. image   */ 
  ipFloat8_t      smin, smax;       /* product of extreme greyvalues    */


  /* check image data                                                   */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues for data type of images*/

  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
    return ( mitkIpFuncERROR );

  /* find out data type of new iamge                                    */

  if ( keep == ipFuncKeep )
    {
       pic_new = ipPicCopyHeader ( pic_old, NULL );
       if ( pic_new == NULL ) return ( mitkIpFuncERROR );
    }
  else if ( keep == ipFuncNoKeep )
    {
       /* calculate max. and min. greyvalues of both images                  */

       if ( ipFuncExtr ( pic_old, &min1, &max1 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

       smax      = pic_old->n[2] * max1;
       smin      = pic_old->n[2] * min1;

       /* change image type of images of type ipPicInt                  */

       if ( pic_old->type == ipPicInt )
         {
           if ( smax < max_gv && smin > min_gv ) 
             {
                pic_new = ipPicCopyHeader ( pic_old, NULL );
             }
           else
             {
                pic_new       = ipPicCopyHeader ( pic_old, NULL );
                pic_new->type = ipPicFloat;
                pic_new->bpe  = 64;
                _ipFuncExtT ( pic_new->type, pic_new->bpe, &min_gv, &max_gv );
             }
         }

       /* change image type of images of type ipPicUInt                 */

       else if ( pic_old->type == ipPicUInt )
         {
           if ( smax < max_gv && smin > min_gv )
             {
                pic_new = ipPicCopyHeader ( pic_old, NULL );
             }
           else
             {
                pic_new = ipPicCopyHeader ( pic_old, NULL );
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
 
       else if ( pic_old->type == ipPicFloat )
         {
            pic_new = ipPicCopyHeader ( pic_old, NULL );
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
  pic_new->dim  = 2;
  pic_new->n[2] = 0;
  pic_new->data = calloc ( _ipPicElements  ( pic_new ), pic_new->bpe/8 );
  if ( pic_new->data == NULL )
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* macro to invert the picture (for all data types)                   */

  if ( keep == ipFuncNoKeep )
    mitkIpPicFORALL_1 ( ADDSL, pic_old, pic_new )            
  else if ( keep == ipFuncKeep )
    mitkIpPicFORALL_1 ( ADDSL3, pic_old, pic_new )                

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
    


  return pic_new;
}
#endif

