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

/*
* internal only. used by Scale.c
*/
/*@file
 *  this function scales the first image to the size of the second image 
 *  using next-neighbour interpolation 
 */

/* @brief scales the first image to the size of the second image 
 *  using next-neighbour interpolation 
 *
 *  @param pic_old      pointer to the image that should be scaled 
 *  @param pic_new      pointer to the scaled image ( contains the information 
 *                for the scaling in th header )
 *                !!! Header must contain all information when calling !!!
 *                !!! function                                         !!!
 *
 * @return  pointer to the scaled image 
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   
 
mitkIpPicDescriptor *_mitkIpFuncScNN( mitkIpPicDescriptor *pic_old,           
                             mitkIpPicDescriptor *pic_new ) ;

#ifndef DOXYGEN_IGNORE

/* definition of macros                                                 */

#define SCNN4ALL( type, pic_old, pic_new, size, weights, n, shift, step )\
{                                                                        \
   size_t           ind[_mitkIpPicNDIM];      /* loop index vector         */\
   size_t           offset[_mitkIpPicNDIM];   /* offset vector             */\
   size_t           i;                    /* loop index                */\
   size_t           off_new;              /* offset of scaled image    */\
   size_t           off_orig;             /* offset of orig. image     */\
                                                                         \
                                                                         \
   /* calculate offsets for scaling an image                          */ \
                                                                         \
   off_new = 0;                                                          \
   for ( ind[7] = 0; ind[7] < n[7]; ind[7]++ )                           \
     {                                                                   \
       offset[7] = ( mitkIpUInt4_t ) ( ( mitkIpFloat8_t ) ind[7] *               \
                     weights[7] + 0.5 ) * size[7];                       \
       for ( ind[6] = 0; ind[6] < n[6]; ind[6]++ )                       \
         {                                                               \
           offset[6] = ( mitkIpUInt4_t ) ( ( mitkIpFloat8_t ) ind[6] *           \
                         weights[6] + 0.5 ) * size[6];                   \
           for ( ind[5] = 0; ind[5] < n[5]; ind[5]++ )                   \
             {                                                           \
               offset[5] = ( mitkIpUInt4_t )  ( ( mitkIpFloat8_t ) ind[5] *      \
                             weights[5] + 0.5 ) * size[5];               \
               for ( ind[4] = 0; ind[4] < n[4]; ind[4]++ )               \
                 {                                                       \
                   offset[4] = ( mitkIpUInt4_t ) ( ( mitkIpFloat8_t ) ind[4] *   \
                                 weights[4] + 0.5 ) * size[4];           \
                   for ( ind[3] = 0; ind[3] < n[3]; ind[3]++ )           \
                     {                                                   \
                       offset[3] = ( mitkIpUInt4_t ) ( ( mitkIpFloat8_t ) ind[3]*\
                                     weights[3] + 0.5 ) * size[3];       \
                       for ( ind[2] = 0; ind[2] < n[2]; ind[2]++ )       \
                         {                                               \
                            offset[2] = ( mitkIpUInt4_t )                    \
                                        ( ( mitkIpFloat8_t ) ind[2] *        \
                                        weights[2] + 0.5 ) * size[2];    \
                           for ( ind[1] = 0; ind[1] < n[1]; ind[1]++ )   \
                             {                                           \
                               offset[1] = ( mitkIpUInt4_t )                 \
                                           ( ( mitkIpFloat8_t ) ind[1] *     \
                                           weights[1]+0.5 ) * size[1];   \
                               for ( ind[0] = 0; ind[0] < n[0]; ind[0]++)\
                                 {                                       \
                                   offset[0] = ( mitkIpUInt4_t )             \
                                               ( ( mitkIpFloat8_t ) ind[0] * \
                                               weights[0] + 0.5)*size[0];\
                                   off_orig = 0;                         \
                                   for ( i = 0; i < pic_old->dim; i++ )  \
                                     off_orig = off_orig + offset[i];    \
                                                                         \
                                   (( type * )pic_new->data)[off_new+shift]=\
                                     (( type * )pic_old->data)[off_orig+shift];\
                                                                         \
                                   off_new+=step;                        \
                                 }                                       \
                             }                                           \
                         }                                               \
                     }                                                   \
                 }                                                       \
             }                                                           \
         }                                                               \
     }                                                                   \
}

#define SCNN( type, pic_old, pic_new, size, weights, n )                 \
		SCNN4ALL( type, pic_old, pic_new, size, weights, n, 0, 1 )
#define SCNNCOLOR( type, pic_old, pic_new, size, weights, n )            \
		SCNN4ALL( type, pic_old, pic_new, size, weights, n, 0, 3 )       \
		SCNN4ALL( type, pic_old, pic_new, size, weights, n, 1, 3 )       \
		SCNN4ALL( type, pic_old, pic_new, size, weights, n, 2, 3 )
/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

mitkIpPicDescriptor *_mitkIpFuncScNN( mitkIpPicDescriptor *pic_old,           
                             mitkIpPicDescriptor *pic_new ) 
{

  mitkIpUInt4_t       i;               /* loopindex                      */
  mitkIpUInt4_t       n[_mitkIpPicNDIM];      /* no. of pixels in each dimension*/
  mitkIpFloat8_t      weights[_mitkIpPicNDIM];
  mitkIpUInt4_t       size[_mitkIpPicNDIM];                                    
  char            is_color=0;


  /* check whether data are correct                                     */

  if(pic_old->bpe==24)
  {
	  is_color=1;
	  pic_old->bpe=pic_new->bpe=8;
  }

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _mitkIpFuncError ( pic_new ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( pic_old->dim != pic_new->dim ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of vectors                                          */
   
  for ( i = 0; i < pic_new->dim; i++ )                                    
    n[i] = pic_new->n[i];                                                 
                                                                          
  for ( i = pic_new->dim; i < _mitkIpPicNDIM; i++ )                           
    n[i] = 1;

  size[0] = 1*(is_color!=0?3:1);
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];

  for ( i = 0; i < pic_old->dim; i++ )
    weights[i] = ( mitkIpFloat8_t ) ( pic_old->n[i] - 1 ) / 
                 ( mitkIpFloat8_t ) ( pic_new->n[i] - 1 );
 
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    weights[i] = 0.;
  
  /* add header information for pic_new and allocate memory             */

  pic_new->bpe  = pic_old->bpe*(is_color!=0?3:1);
  pic_new->type = pic_old->type;
  pic_new->data = malloc ( _mitkIpPicSize ( pic_new ) );

  if ( pic_new->data == NULL )
    {
       mitkIpPicFree ( pic_new );
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );                   
       return ( mitkIpFuncERROR );
    }
 
  /* macro to scale an image                                            */
  if(is_color==0)
  {
	  mitkIpPicFORALL_4 ( SCNN, pic_old, pic_new, size, weights, n ); 
  }
  else
  {
	  mitkIpPicFORALL_4 ( SCNNCOLOR, pic_old, pic_new, size, weights, n ); 
	  pic_old->bpe=pic_new->bpe=24;
  }

  return pic_new;
}
#endif
