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
 *  this function converts one image to an image of another data type
 *  (at the moment only datatype ipUInt1_t, ipInt2_t, ipFloat8_t ) are
 *  supported.
 */
 
 /**  this function converts one image to an image of another data type
 *  (at the moment only datatype ipUInt1_t, ipInt2_t, ipFloat8_t ) are
 *  supported.
 *
 *  @param pic_old      pointer to input image
 *  @param type         image data type 
 *                    3    ipPicInt
 *                    4    ipPicUInt
 *                    5    ipPicFloat
 *  @param bpe          bits per element representing on pixel
 *
 *  qreturn pointer to converted image
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"


ipPicDescriptor *ipFuncConvert ( ipPicDescriptor *pic_old,
                                 ipPicType_t     type,
                                 ipUInt4_t       bpe );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncConvert\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/* definition of macros                                                 */

#define CONV_1( type_old, pic_old, pic_new, min_gv, max_gv )             \
{                                                                        \
  mitkIpPicFORALL_4( CONV_2, pic_new, pic_old, type_old, min_gv, max_gv )    \
}
 
#define CONV_3( type_old, pic_old, pic_new )                             \
{                                                                        \
  mitkIpPicFORALL_2( CONV_4, pic_new, pic_old, type_old )                    \
}
 

#define CONV_4( type_new, pic_new, pic_old, type_old )                   \
{                                                                        \
  size_t  i;                                                          \
  size_t  no_elem;                                                    \
                                                                         \
  no_elem =  _ipPicElements ( pic_old );                                 \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
       (( type_new * ) pic_new->data ) [i] = ( type_new )                \
               (( type_old * ) pic_old->data ) [i];                      \
    }                                                                    \
}

#define CONV_2( type_new, pic_new, pic_old, type_old, min_gv, max_gv )   \
{                                                                        \
  size_t  i;                                                          \
  size_t  no_elem;                                                    \
  type_old   help;                                                       \
                                                                         \
  no_elem =  _ipPicElements ( pic_new );                                 \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
       help  = (( type_old * ) pic_old->data ) [i];                      \
       (( type_new * ) pic_new->data ) [i] =                             \
          ( max_gv > ( ipFloat8_t ) help ) ?                             \
             ( min_gv < ( ipFloat8_t ) help  ?                           \
                 ( type_new )help : ( type_new ) min_gv ) :              \
             ( type_new ) max_gv;                                        \
    }                                                                    \
}



/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncConvert ( ipPicDescriptor *pic_old,
                                 ipPicType_t     type,
                                 ipUInt4_t       bpe )

{
  ipFloat8_t      min, max;         /* extreme greyvalues of 1. image   */ 
  ipFloat8_t      max_gv;
  ipFloat8_t      min_gv;           /* min. possible greyvalue          */
  ipPicDescriptor *pic_new;         /* pointer to new image             */


  /* ckeck whether data are correct                                     */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if ( ! ( ( type == ipPicFloat && ( bpe == 32 || bpe == 64 ) ) ||
           ( type == ipPicUInt  && ( bpe == 8 || bpe == 16 || bpe == 32 ) ) ||
           ( type == ipPicInt   && ( bpe == 8 || bpe == 16 || bpe == 32 ) )  ) )
     {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
     }
           
  /* calculate max. and min. possible greyvalues for output image       */

  if ( _ipFuncExtT ( type, bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
    return ( mitkIpFuncERROR );

  /* calculate extreme greyvalues in input image                        */

  if ( ipFuncExtr ( pic_old, &min, &max ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* allocate output image                                              */

  pic_new       = ipPicCopyHeader ( pic_old, NULL );
  if ( pic_new == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }
  pic_new->type = type;
  pic_new->bpe  = bpe;

  pic_new->data = malloc ( _ipPicSize  ( pic_new ) );
  if ( pic_new->data == NULL )
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* macro to convert the picture (for all data types)                  */

  if ( ( min < min_gv ) || ( max > max_gv ) )
    mitkIpPicFORALL_3 ( CONV_1, pic_old, pic_new, min_gv, max_gv )
  else 
    mitkIpPicFORALL_1 ( CONV_3, pic_old, pic_new )

  ipFuncCopyTags(pic_new, pic_old);
  
      

  return pic_new;
}
#endif

