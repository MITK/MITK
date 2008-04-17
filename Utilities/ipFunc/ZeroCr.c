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
 *  this function detects zero-crossings
 */

/** @brief detects zero-crossings              
 *
 *  @param pic_old     pointer to the image that should be transformed
 *
 *  @return pointer to the transformed image 
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"   
 
ipPicDescriptor *ipFuncZeroCr ( ipPicDescriptor *pic_old ) ;

#ifndef DOXYGEN_IGNORE

/* definition of macros                                                 */

#define ZERO( type, pic, size, off, n )                                  \
{                                                                        \
   ipUInt4_t        ind[_mitkIpPicNDIM];                                     \
   ipUInt4_t        i, j;                                                \
   ipInt4_t         offset;                                              \
   type             help1, help2, help3, help;                           \
                                                                         \
   /* test whether there is a zero crossing in each direction         */ \
                                                                         \
   for ( ind[7] = 1; ind[7] < n[7]; ind[7]++ )                           \
     for ( ind[6] = 1; ind[6] < n[6]; ind[6]++ )                         \
       for ( ind[5] = 1; ind[5] < n[5]; ind[5]++ )                       \
         for ( ind[4] = 1; ind[4] < n[4]; ind[4]++ )                     \
           for ( ind[3] = 1; ind[3] < n[3]; ind[3]++ )                   \
             for ( ind[2] = 1; ind[2] < n[2]; ind[2]++ )                 \
               for ( ind[1] = 1; ind[1] < n[1]; ind[1]++ )               \
                 for ( ind[0] = 1; ind[0] < n[0]; ind[0]++ )             \
                   {                                                     \
                      offset = 0;                                        \
                      for ( i = 0; i < pic->dim; i++ )                   \
                        offset = offset + size[i] * ind[i];              \
                                                                         \
                       i = 0;                                            \
                       help = (( type * )pic_new->data )[offset];        \
                       help2 = ((type *)pic->data)[offset];              \
                       while ( help == 0 && i < pic->dim )               \
                         {                                               \
                            j = 2 * i;                                   \
                            help1 = ((type *)pic->data)[offset + off[j]];\
                            help3 = ((type *)pic->data)[offset+off[j+1]];\
                                                                         \
                            if ( help1 < 0 && help3 > 0 )                \
                              {                                          \
                                 if ( help2 > 0 && (help1 + help2) <=0 ) \
                                   help = ( type ) 1;                    \
                                 else if ( help2<=0 && (help2+help3) >0 )\
                                   help = ( type ) 1;                    \
                              }                                          \
                            else if ( help1 > 0 && help3 < 0 )           \
                              {                                          \
                                 if ( help2 >= 0 && (help2 + help3) <0 ) \
                                   help = ( type ) 1;                    \
                                 else if ( help2<0 && (help1+help2) >=0 )\
                                   help = ( type ) 1;                    \
                              }                                          \
                            i++;                                         \
                         }                                               \
                       (( type * )pic_new->data )[offset] = help;        \
                                                                         \
                    }                                                    \
}

               

/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncZeroCr ( ipPicDescriptor *pic_old )          
{

  ipPicDescriptor *pic_new;           /* convoluted image               */
  ipUInt4_t       i, j;               /* loopindex                      */
  ipInt4_t        size[_mitkIpPicNDIM];
  ipUInt4_t       n[_mitkIpPicNDIM];
  ipInt4_t        *off_vekt;          /* offset vector                  */

  /* check image data                                                   */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* initialisation of vectors                                          */
  
  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];
                                                                          
  for ( i = 0; i < pic_old->dim; i++ )           
    n[i] = pic_old->n[i] - 1;                                              
                                                                          
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )                    
    n[i] = 2;                                                             
                                                                          
  /* allocate and calculate offset vector                               */

  off_vekt = malloc ( 2 * pic_old->dim * sizeof ( ipInt4_t ) );
  if ( off_vekt == NULL ) 
    {  
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  for ( i = 0; i < pic_old->dim; i++ )
    {
       j = 2 * i;
       off_vekt[j]   = - size[i];
       off_vekt[j+1] =   size[i];
    } 


  /* create a new picture, copy the header, allocate memory             */

  pic_new = ipPicCopyHeader ( pic_old, 0 );
  if ( pic_new == NULL )
    {
       free ( off_vekt );
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );                
       free ( off_vekt );
       return ( mitkIpFuncERROR );
    }
  pic_new->data = calloc ( _ipPicElements ( pic_new ), pic_new->bpe/8  );
  if ( pic_new->data == NULL )
    {
       free ( off_vekt );
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );                
       free ( off_vekt );
       return ( mitkIpFuncERROR );
    }

 
  /* macro to convolute an image (for all data types)                   */

  mitkIpPicFORALL_3 ( ZERO, pic_old, size, off_vekt, n );                    

  free ( off_vekt );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  
                        

  return pic_new;
}
#endif
