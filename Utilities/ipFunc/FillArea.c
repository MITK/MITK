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
 *  this function fills an area
 */

/** @brief fills an area
 *
 *  @warning not tested
 *
 * AUTHOR & DATE
 */

#ifndef lint
  static char *what = { "@(#)\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* include-Files                                                        */

#include "mitkIpFuncP.h"   
 
/* definition of macros and constants                                   */

#define KL <
#define GR > 

#define MAX( x, y ) ( x > y ) ? x : y  
#define MIN( x, y ) ( x < y ) ? x : y  

#define RECT( type, pic, beg, end, value )                               \
{                                                                        \
  ipUInt4_t  i, j, n;                                                    \
  ipUInt4_t  offset;                                                     \
                                                                         \
  n = pic->n[0];                                                         \
  for ( j = beg[1]; j < end[1]; j++ )                                    \
    {                                                                    \
        offset = j * n;                                                  \
        for ( i = beg[0]; i < end[0]; i++ )                              \
         (( type * )pic->data )[i + offset] = ( type ) value;            \
    }                                                                    \
} 
#define TRI( type, pic, beg, end, value, a, b, OP )                      \
{                                                                        \
  ipUInt4_t  i, j, n;                                                    \
  ipUInt4_t  offset;                                                     \
  ipFloat8_t help;                                                       \
                                                                         \
  n = pic->n[0];                                                         \
  for ( j = beg[1]; j < end[1]; j++ )                                    \
    {                                                                    \
       offset = j * n;                                                   \
       for ( i = beg[0]; i < end[0]; i++ )                               \
         {                                                               \
           help = a * i + b;                                             \
           if ( j OP help )                                              \
             {                                                           \
                (( type * )pic->data )[i + offset] = ( type ) value;     \
             }                                                           \
         }                                                               \
    }                                                                    \
} 
               
 

/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

mitkIpPicDescriptor *mitkIpFuncFillArea ( mitkIpPicDescriptor *pic_old,
                                  mitkIpFuncBox_t     box,
                                  ipFloat8_t      value, 
                                  mitkIpFuncFlagI_t   over,
                                  mitkIpPicDescriptor *pic_return )
{

  mitkIpPicDescriptor *pic_new;  /* inverted picture                        */
  ipFloat8_t      a, b;      /* parameter der Gerade y = ax + b         */
  ipUInt4_t       beg[2];    /* Anfangswerte                            */
  ipUInt4_t       end[2];    /* Endwerte                                */


  /* check data                                                         */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* create a new picture, copy the header, allocate memory             */

  pic_new = _mitkIpFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );     
  if ( pic_new == NULL ) return ( mitkIpFuncERROR );
  if ( pic_new != pic_old )
    {
       memcpy ( pic_new->data, pic_old->data, _mitkIpPicElements ( pic_old ) * pic_old->bpe /8 );
    }

  /* Gerade durch die beiden Punkte                                     */
  
  a = ( ( ipFloat8_t ) box.y1 - ( ipFloat8_t ) box.y0 ) / 
      ( ( ipFloat8_t ) box.x1 - ( ipFloat8_t ) box.x0 );

  b = ( ipFloat8_t ) box.y0 - a *  box.x0;

  /* check which region (above/beneath) has to be changed               */

  if ( over == mitkIpFuncAbove )
    {
        beg[0] = 0; 	    end[0] = pic_old->n[0];
        beg[1] = 0; 	    end[1] = MIN ( box.y0, box.y1 );
     /* printf ( "1.Rechteck: beg %d %d end: %d %d \n", beg[0], beg[1], end[0], end[1] ); */
        mitkIpPicFORALL_3 ( RECT, pic_new, beg, end, value );

        beg[0] = ( box.y0 < box.y1 ) ? box.x1 : 0;
        end[0] = ( box.y0 < box.y1 ) ? pic_old->n[0] : box.x0;
        beg[1] = MIN ( box.y0, box.y1 );
        end[1] = MAX ( box.y0, box.y1 );
/*      printf ( "2.Rechteck: beg %d %d end: %d %d \n", beg[0], beg[1], end[0], end[1] ); */
        mitkIpPicFORALL_3 ( RECT, pic_new, beg, end, value );

        beg[0] = box.x0;    end[0] = box.x1; 
        beg[1] = MIN ( box.y0, box.y1 );
        end[1] = MAX ( box.y0, box.y1 );
/*      printf ( "1.Dreieck:  beg %d %d end: %d %d \n", beg[0], beg[1], end[0], end[1] ); */
        mitkIpPicFORALL_6 ( TRI, pic_new, beg, end, value, a, b, KL );
    }
  else if ( over == mitkIpFuncBeneath )
    {
        beg[0] = 0;                          end[0] = pic_old->n[0];
        beg[1] = MAX ( box.y0, box.y1 ); end[1] = pic_old->n[1];
/*      printf ( "1.Rechteck: beg %d %d end: %d %d \n", beg[0], beg[1], end[0], end[1] ); */
        mitkIpPicFORALL_3 ( RECT, pic_new, beg, end, value );

        beg[0] = ( box.y0 < box.y1 ) ? 0 : box.x1;
        end[0] = ( box.y0 < box.y1 ) ? box.x0 : pic_old->n[0]; 
        beg[1] = MIN ( box.y0, box.y1 );
        end[1] = MAX ( box.y0, box.y1 );
/*      printf ( "2.Rechteck: beg %d %d end: %d %d \n", beg[0], beg[1], end[0], end[1] ); */
        mitkIpPicFORALL_3 ( RECT, pic_new, beg, end, value );

        beg[0] = box.x0;    end[0] = box.x1; 
        beg[1] = MIN ( box.y0, box.y1 );
        end[1] = MAX ( box.y0, box.y1 );
/*      printf ( "1.Dreieck:  beg %d %d end: %d %d \n", beg[0], beg[1], end[0], end[1] ); */
        mitkIpPicFORALL_6 ( TRI, pic_new, beg, end, value, a, b, GR );
    }
  else
    {
       _mitkIpFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );   
    }

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_old);
  
  



  return pic_new;
}

