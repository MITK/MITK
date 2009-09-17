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
 *  this function sets the greyvalues of all pixels outside a given
 *  range to the an extremal greyvalue
 */

/** @brief sets the greyvalues of all pixels outside a given
 *  range to the an extremal greyvalue

 *  Sets the greyvalue of pixels that are smaller than gv_low to this value and
 *  those larger than gv_up to that value. Very similar to
 *  and used by mitkIpFuncLevWin().
 *
 *  @param pic_old    pointer to original image
 *  @param gv_low     lower greyvalue of range
 *  @param gv_up      upper greyvalue of range
 *  @param pic_return memory used to store return image ( if pic_return == NULL
 *                new memory is allocated )
 *
 * @return pointer to transformed image
 *
 * AUTHOR & DATE
 */

/* include files                                                            */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncSelMM ( mitkIpPicDescriptor *pic_old,
                               mitkIpFloat8_t      gv_low,
                               mitkIpFloat8_t      gv_up, 
                               mitkIpPicDescriptor *pic_return ) ;

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                     */

#define SELECT( type, pic, gv_low, gv_up, min_gv, max_gv )                   \
{                                                                            \
  mitkIpUInt4_t    i;                 /* loopindex                             */\
  mitkIpUInt4_t    no_elem;           /* loopindex                             */\
  type         help;              /* element that should be transformed    */\
                                                                             \
  /* transform greyvalues                                                  */\
                                                                             \
  no_elem = _mitkIpPicElements ( pic );                                          \
  for ( i = 0; i < no_elem; i++ )                                            \
    {                                                                        \
       help = (( type * )pic->data )[i];                                     \
       (( type * )pic_new->data )[i] =                                       \
          ( help < ( type ) gv_up ) ?                                        \
            (( help > ( type ) gv_low ) ? help : ( type ) gv_low  ) :        \
            ( type ) gv_up ;                                                 \
    }                                                                        \
}                                                                             

/* ------------------------------------------------------------------------ */
/*
*/
/* ------------------------------------------------------------------------ */

mitkIpPicDescriptor *mitkIpFuncSelMM ( mitkIpPicDescriptor *pic_old,
                               mitkIpFloat8_t      gv_low,
                               mitkIpFloat8_t      gv_up, 
                               mitkIpPicDescriptor *pic_return ) 
{
  mitkIpFloat8_t       min_gv, max_gv;  /* max and min possible greyvalues      */
  mitkIpPicDescriptor  *pic_new;        /* pointer to transformed image         */


  /* calculate max. and min. possible greyvalues                            */

  if ( _mitkIpFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) == mitkIpFuncERROR )
    return ( mitkIpFuncERROR );

  /* check whether data are correct                                         */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if ( gv_low >= gv_up )  
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  if ( min_gv > gv_low || max_gv < gv_up ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* allocate memory for the transformed image                              */

   pic_new = _mitkIpFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );
   if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* macro to transform the image                                           */

  mitkIpPicFORALL_4 ( SELECT, pic_old, gv_low, gv_up, min_gv, max_gv );

  mitkIpFuncCopyTags(pic_new, pic_old);
  
     

  return ( pic_new );
}
#endif

