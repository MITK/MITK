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
 *    this function calculates the extreme greyvalues of an image
 */


/**    this function calculates the extreme greyvalues of an image
 *  @param    pic    pointer to the image 
 *  @param    &min   result variable: minimum   
 *  @param    &max   result variable: maximum  
 *  
 *  @return    mitkIpFuncOK    no error occured
 *  @return    mitkIpFuncERROR  an error occured
 *
 * AUTHOR
 */

/* include-files                                              */

#include "mitkIpFuncP.h"

ipInt4_t mitkIpFuncExtr ( mitkIpPicDescriptor *pic,
                      ipFloat8_t      *min, 
                      ipFloat8_t      *max );

#ifndef DOXYGEN_IGNORE


#ifndef lint
  static char *what = { "@(#)mitkIpFuncExtr\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



 
/* definition of extreme value macro                          */

#define EXTR( type, pic, min_c, max_c )                        \
  {                                                            \
    ipUInt4_t   i;                                             \
    ipUInt4_t   no_elem;                                       \
    ipFloat8_t  help;                                          \
                                                               \
    no_elem = _mitkIpPicElements ( pic );                          \
    max_c = ( ipFloat8_t )( ( type * ) pic->data ) [0];        \
    min_c = ( ipFloat8_t )( ( type * ) pic->data ) [0];        \
                                                               \
    for ( i = 1; i < no_elem; i++ )                            \
      {                                                        \
        help   = ( ipFloat8_t )( ( type * ) pic->data ) [i];   \
        max_c  = ( help > max_c ) ? help : max_c;              \
        min_c  = ( help < min_c ) ? help : min_c;              \
                                                               \
      }                                                        \
  }
 
/* ========================================================== */
/*
*/
/* ========================================================== */

ipInt4_t mitkIpFuncExtr ( mitkIpPicDescriptor *pic,
                      ipFloat8_t      *min, 
                      ipFloat8_t      *max )
{

  ipFloat8_t min_c = 0.0;            
  ipFloat8_t max_c = 0.0;

  /* check data                                               */

  if ( _mitkIpFuncError ( pic ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* macro to calculate extreme values ( for all data types   */

  mitkIpPicFORALL_2( EXTR, pic, min_c, max_c)

  *min = min_c;
  *max = max_c;

  return( mitkIpFuncOK );
}
#endif

