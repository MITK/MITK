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
 *  this function copies a rectangular section of an image to a new one 
 */

/** @brief copies a rectangular section of an image to a new one 
 *
 *  @param pic_old     pointer to the original image
 *  @param begin       pointer to an array that contains the beginning of the   
 *               section in each direction
 *  @param length      pointer to an array that contains the length of the section  
 *               in each direction
 *
 * @return pointer to the iamge with the section of the iamge
 *
 * AUTHOR & DATE
 */

/* include files                                                                  */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncWindow ( ipPicDescriptor *pic_old,
                                ipUInt4_t       *begin,
                                ipUInt4_t       *length );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* definition of macros                                                           */


#define WIND( type, pic, beg, end, size )                                          \
{                                                                                  \
  ipUInt4_t      offset;           /* offset of pixels in pic_new               */ \
  ipUInt4_t      ind[_mitkIpPicNDIM];  /* loop index vector                         */ \
  ipUInt4_t      off[_mitkIpPicNDIM];  /* used to calculate offset of image pixels  */ \
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
                  (( type * ) pic_new->data )[offset] =                            \
                     (( type * ) pic->data ) [off[0]];                             \
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

ipPicDescriptor *ipFuncWindow ( ipPicDescriptor *pic_old,
                                ipUInt4_t       *begin,
                                ipUInt4_t       *length )
{
  ipPicDescriptor *pic_new;            /* pointer to transformed image            */
  ipUInt4_t      i;                    /* loop index                              */
  ipUInt4_t      end[_mitkIpPicNDIM];      /* end of image                            */
  ipUInt4_t      beg[_mitkIpPicNDIM];      /* end of image                            */
  ipUInt4_t      size[_mitkIpPicNDIM];     /*                                         */

  /* check whether data are correct                                               */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  for ( i = 0; i < pic_old->dim; i++ )
    {
      if ( begin[i] < 0 || begin[i] > pic_old->n[i] ) 
        {  
           _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
           return ( mitkIpFuncERROR );
        }
      beg[i] = begin[i];
      end[i] = begin[i] + length[i];
      if ( length[i] < 0 || end[i] > pic_old->n[i] )
        {  
           _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
           return ( mitkIpFuncERROR );
        }
    }

  /* initialize vectors and variables                                             */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];
  size[pic_old->dim] = 0;


  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    {
       beg[i] = 0;
       end[i] = 1;
    }

  /* allocate image structure                                                     */

  pic_new = ipPicNew ( );

  if ( pic_new == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );                    
       return ( mitkIpFuncERROR );
    }

  pic_new->dim  = pic_old->dim;
  pic_new->type = pic_old->type;
  pic_new->bpe  = pic_old->bpe;
  for ( i = 0; i < pic_new->dim; i++ )
    pic_new->n[i] = length[i];
  pic_new->data = malloc ( _ipPicSize ( pic_new ) );
  if ( pic_new->data == NULL )
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );                    
       return ( mitkIpFuncERROR );
    }

  mitkIpPicFORALL_3 ( WIND, pic_old, beg, end, size );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  
                        
  return ( pic_new );
}

#endif

