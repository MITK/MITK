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
 *  this function copies an image to a specified position in another image
 */

/** @brief copies an image to a specified position in another image
 *
 *  @param pic_1      pointer to the image into which @a pic_2 should be inserted
 *  @param pic_2      pointer to image that should be inserted
 *  @param begin      pointer to an array that contains  the position at which    
 *               @a pic_2 ist inserted to @a pic_1
 *
 * @return pointer to the new iamge 
 *
 * AUTHOR & DATE
 */

/* include files                                                                  */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncWindowR ( mitkIpPicDescriptor *pic_1,  
                                 mitkIpPicDescriptor *pic_2,
                                 mitkIpUInt4_t       *begin,
                                 mitkIpFuncFlagI_t    keep );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                           */


#define WINDR( type, pic, beg, end, size )                                         \
{                                                                                  \
  mitkIpUInt4_t      offset;           /* offset of pixels in pic_new               */ \
  mitkIpUInt4_t      ind[_mitkIpPicNDIM];  /* loop index vector                         */ \
  mitkIpUInt4_t      off[_mitkIpPicNDIM];  /* used to calculate offset of image pixels  */ \
                                                                                   \
  offset = 0;                                                                      \
  for ( ind[7] = beg[7] ; ind[7] < end[7]; ind[7]++ )                              \
  {                                                                                \
    off[7] = size[7] * ind[7];                                                     \
    for ( ind[6] = beg[6] ; ind[6] < end[6]; ind[6]++ )                            \
    {                                                                              \
      off[6] = size[6] * ind[6] + off[7];                                          \
      for ( ind[5] = beg[5] ; ind[5] < end[5]; ind[5]++ )                          \
      {                                                                            \
        off[5] = size[5] * ind[5] + off[6];                                        \
        for ( ind[4] = beg[4] ; ind[4] < end[4]; ind[4]++ )                        \
        {                                                                          \
          off[4] = size[4] * ind[4] + off[5];                                      \
          for ( ind[3] = beg[3] ; ind[3] < end[3]; ind[3]++ )                      \
          {                                                                        \
            off[3] = size[3] * ind[3] + off[4];                                    \
            for ( ind[2] = beg[2] ; ind[2] < end[2]; ind[2]++ )                    \
            {                                                                      \
              off[2] = size[2] * ind[2] + off[3];                                  \
              for ( ind[1] = beg[1] ; ind[1] < end[1]; ind[1]++ )                  \
              {                                                                    \
                off[1] = size[1] * ind[1] + off[2];                                \
                off[0] = off[1]+beg[0];                                            \
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )                 \
                {                                                                  \
                  (( type * ) pic_new->data )[off[0]] =                            \
                     (( type * ) pic->data ) [offset];                             \
                  off[0]++;                                                        \
                  offset++;                                                        \
                }                                                                  \
              }                                                                    \
            }                                                                      \
          }                                                                        \
        }                                                                          \
      }                                                                            \
    }                                                                              \
  }                                                                                \
}


/* ------------------------------------------------------------------------------ */
/*
*/
/* ------------------------------------------------------------------------------ */

mitkIpPicDescriptor *mitkIpFuncWindowR ( mitkIpPicDescriptor *pic_1,  
                                 mitkIpPicDescriptor *pic_2,
                                 mitkIpUInt4_t       *begin,
                                 mitkIpFuncFlagI_t    keep )
{
  mitkIpPicDescriptor *pic_new;            /* pointer to transformed image            */
  mitkIpInt4_t       i;                    /* loop index                              */
  mitkIpUInt4_t      end[_mitkIpPicNDIM];      /* end of image                            */
  mitkIpUInt4_t      size[_mitkIpPicNDIM];     /*                                         */

  /* check whether data are correct                                               */

  if ( _mitkIpFuncError ( pic_1 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _mitkIpFuncError ( pic_2 ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if ( pic_1->dim != pic_2->dim ) 
     {
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
     }
  if ( pic_1->type != pic_2->type ) 
     {
       _mitkIpFuncSetErrno ( mitkIpFuncTYPE_ERROR );
       return ( mitkIpFuncERROR );
     }
  if ( pic_1->bpe != pic_2->bpe )
     {
       _mitkIpFuncSetErrno ( mitkIpFuncSIZE_ERROR );
       return ( mitkIpFuncERROR );
     }
  
  for ( i = 0; i < pic_1->dim; i++ )
    {
      if ( begin[i] < 0 || begin[i] > pic_1->n[i] ) 
        {
           _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
           return ( mitkIpFuncERROR );
        }
      end[i] = begin[i] + pic_2->n[i];
      if ( end[i] > pic_1->n[i] )
        {
           _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
           return ( mitkIpFuncERROR );
        }
    }

  /* initialize vectors and variables                                             */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_1->n[i-1];
  size[pic_1->dim] = 0;


  for ( i = pic_1->dim; i < _mitkIpPicNDIM; i++ )
    {
       begin[i] = 0;
       end[i]   = 1;
    }

  /* allocate image structure                                                     */

  if ( keep == mitkIpFuncKeep )
     pic_new = mitkIpPicClone ( pic_1 ); 
  else 
     pic_new = pic_1;

  mitkIpPicFORALL_3 ( WINDR, pic_2, begin, end, size );

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_1);
                        
  return ( pic_new );
}
#endif

