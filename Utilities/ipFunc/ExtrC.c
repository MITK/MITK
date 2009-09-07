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
 *  this function calculates the extreme greyvalues of all greyvalues which are
 *  included by a circle.
 */

/** @brief calculates the extreme greyvalues of all greyvalues which are
 *  included by a circle.
 *
 *  The circle is described by its center and radius.
 *
 *  @param pic_old  pointer to the original image
 *  @param min`  pointer to minimal greyvalue in circle ( return value )
 *  @param max   pointer to maximal greyvalue in circle ( return value )
 *  @param center  pointer to an array that contains the coordinates 
 *               of the center 
 *               of the circle
 *  @param radius  radius of the circle                   
 *
 *  @return mitkIpFuncOK       - if no error occured
 *  @return mitkIpFuncERROR    - if an error occured
 *
 * AUTHOR & DATE
 */

/* include files                                                                  */

#include "mitkIpFuncP.h"

ipUInt4_t  mitkIpFuncExtrC ( mitkIpPicDescriptor *pic_old,
                         ipFloat8_t      *min,
                         ipFloat8_t      *max,
                         ipUInt4_t       *center,
                         ipUInt4_t       radius );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncExtrC\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                           */


#define EXTR( type, pic, beg, end, size, center, radius )                          \
{                                                                                  \
  ipUInt4_t      i;                /* loop index                                */ \
  ipUInt4_t      offset;           /* offset of pixels in pic_new               */ \
  ipUInt4_t      ind[_mitkIpPicNDIM];  /* loop index vector                         */ \
  ipUInt4_t      off[_mitkIpPicNDIM];  /* used to calculate offset of image pixels  */ \
  ipUInt4_t      dist[_mitkIpPicNDIM]; /* used to calculate offset of image pixels  */ \
  ipFloat8_t     help;                                                             \
                                                                                   \
  offset = 0;                                                                      \
  for ( i = 0; i < pic->dim; i++ )                                                 \
    offset = offset + center[i] * size[i];                                         \
  *max = ( ipFloat8_t )( ( type * ) pic->data ) [offset];                          \
  *min = ( ipFloat8_t )( ( type * ) pic->data ) [offset];                          \
                                                                                   \
  for ( ind[7] = beg[7] ; ind[7] < end[7]; ind[7]++ )                              \
  {                                                                                \
    dist[7] = ( ind[7] - center[7] ) * ( ind[7] - center[7] );                     \
    off[7] = size[7] * ind[7];                                                     \
    for ( ind[6] = beg[6] ; ind[6] < end[6]; ind[6]++ )                            \
    {                                                                              \
      dist[6] = ( ind[6] - center[6] ) * ( ind[6] - center[6] ) + dist[7];         \
      off[6] = size[6] * ind[6] + off[7];                                          \
      for ( ind[5] = beg[5] ; ind[5] < end[5]; ind[5]++ )                          \
      {                                                                            \
        dist[5] = ( ind[5] - center[5] ) * ( ind[5] - center[5] ) + dist[6];       \
        off[5] = size[5] * ind[5] + off[6];                                        \
        for ( ind[4] = beg[4] ; ind[4] < end[4]; ind[4]++ )                        \
        {                                                                          \
          dist[4] = ( ind[4] - center[4] ) * ( ind[4] - center[4] ) + dist[5];     \
          off[4] = size[4] * ind[4] + off[5];                                      \
          for ( ind[3] = beg[3] ; ind[3] < end[3]; ind[3]++ )                      \
          {                                                                        \
            dist[3] = ( ind[3] - center[3] ) * ( ind[3] - center[3] ) + dist[4];   \
            off[3] = size[3] * ind[3] + off[4];                                    \
            for ( ind[2] = beg[2] ; ind[2] < end[2]; ind[2]++ )                    \
            {                                                                      \
              dist[2] = ( ind[2] - center[2] ) * ( ind[2] - center[2] ) + dist[3]; \
              off[2] = size[2] * ind[2] + off[3];                                  \
              for ( ind[1] = beg[1] ; ind[1] < end[1]; ind[1]++ )                  \
              {                                                                    \
                dist[1] = ( ind[1] - center[1] ) * ( ind[1] - center[1] ) +dist[2];\
                off[1] = size[1] * ind[1] + off[2];                                \
                off[0] = off[1]+beg[0];                                            \
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )                 \
                {                                                                  \
                  dist[0] = ( ind[0] - center[0] ) * ( ind[0] - center[0] )+dist[1];\
                  if ( sqrt ( ( ipFloat8_t ) dist[0] ) <= radius )                 \
                    {                                                              \
                       help   = ( ipFloat8_t )( ( type * ) pic->data ) [off[0]];   \
                       *max   = ( help > *max ) ? help : *max;                     \
                       *min   = ( help < *min ) ? help : *min;                     \
                    }                                                              \
                  off[0]++;                                                        \
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

ipUInt4_t  mitkIpFuncExtrC ( mitkIpPicDescriptor *pic_old,
                         ipFloat8_t      *min,
                         ipFloat8_t      *max,
                         ipUInt4_t       *center,
                         ipUInt4_t       radius )
{
  ipInt4_t       help;
  ipUInt4_t       i;                   /* loop index                              */
  ipUInt4_t      end[_mitkIpPicNDIM];      /* end of window                           */
  ipUInt4_t      begin[_mitkIpPicNDIM];    /* beginning of window                     */
  ipUInt4_t      centr[_mitkIpPicNDIM];    /* beginning of window                     */
  ipUInt4_t      size[_mitkIpPicNDIM];     /*                                         */

  /* check whether data are correct                                               */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( radius <= 0 ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  for ( i = 0; i < pic_old->dim; i++ )
    {
      help = center[i] - radius;
      if ( help < 0 )
        {
           _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
           return ( mitkIpFuncERROR );
        }
      else begin[i] = ( ipUInt4_t )help;

      help = center[i] + radius;
      if ( (ipUInt4_t) help > pic_old->n[i] )
        {
           _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
           return ( mitkIpFuncERROR );
        }
      else end[i] = ( ipUInt4_t )help + 1;
      centr[i] = center[i];
    }

  /* initialize vectors and variables                                             */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];
  size[pic_old->dim] = 0;

  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    {
       centr[i]  = 0;
       begin[i]  = 0;
       end[i]    = 1;
    }

  /* allocate image structure                                                     */

  mitkIpPicFORALL_5  ( EXTR, pic_old, begin, end, size, centr, radius );
  
  return ( mitkIpFuncOK );    
}

#endif

