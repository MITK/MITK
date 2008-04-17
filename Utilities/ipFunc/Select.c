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
 *  this function sets the greyvalues of all pixels @e outside a given
 *  range to a new greyvalue
 */

/** @brief sets the greyvalues of all pixels @e outside a given
 *  range to a new greyvalue
 *
 *  The greyvalues of all pixels which are @e not included
 *  in [gv_low, gv_up] are set to a new greyvalue (gv).
 *
 *  @param pic_old    pointer to original image
 *  @param gv_low     lower greyvalue of range
 *  @param gv_up      upper greyvalue of range
 *  @param gv         new greyvalue
 *  @param pic_return memory used to store return image ( if pic_return == NULL
 *               new memory is allocated )
 *
 * @return  pointer to transformed image
 *
 * AUTHOR & DATE
 */

/* include files                                                            */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncSelect ( ipPicDescriptor *pic_old,
                                ipFloat8_t      gv_low,
                                ipFloat8_t      gv_up,
                                ipFloat8_t      gv,
                                ipPicDescriptor *pic_return );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                     */

#define SELECT( type, pic, gv_low, gv_up, gv )                               \
{                                                                            \
  ipUInt4_t    i;                 /* loopindex                             */\
  type         help;              /* element that should be transformed    */\
                                                                             \
  /* transform greyvalues                                                  */\
                                                                             \
  for ( i = 0; i < _ipPicElements ( pic ); i++ )                             \
    {                                                                        \
       help = (( type * )pic->data )[i];                                     \
       (( type * )pic_new->data )[i] =                                       \
             ( help < gv_low || help > gv_up ) ?                             \
             /*( help < ( type ) gv_low || help > ( type )gv_up ) ? iw-3.8.2000*/       \
             ( type ) gv : help;                                             \
    }                                                                        \
                                                                             \
}                                                                            \

/* ------------------------------------------------------------------------ */
/*
*/
/* ------------------------------------------------------------------------ */

ipPicDescriptor *ipFuncSelect ( ipPicDescriptor *pic_old,
                                ipFloat8_t      gv_low,
                                ipFloat8_t      gv_up,
                                ipFloat8_t      gv,
                                ipPicDescriptor *pic_return ) 
{
  ipFloat8_t       min_gv, max_gv;  /* max and min possible greyvalues      */
  ipPicDescriptor  *pic_new;        /* pointer to transformed image         */


  /* calculate max. and min. possible greyvalues                            */

  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) == mitkIpFuncERROR )
    return ( mitkIpFuncERROR );

  /* check whether data are correct                                         */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if ( gv_low > gv_up ) 
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
/* iw - commented out on 3.8.2000 
   if ( min_gv > gv_low || max_gv < gv_up ) 
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
*/
  if ( min_gv > gv || max_gv < gv ) 
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* allocate memory for the transformed image                              */


   pic_new = _ipFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );     
   if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* macro to transform the image                                           */

  mitkIpPicFORALL_3 ( SELECT, pic_old, gv_low, gv_up, gv );

  ipFuncCopyTags(pic_new, pic_old);
  
     

  return ( pic_new );
}
#endif
