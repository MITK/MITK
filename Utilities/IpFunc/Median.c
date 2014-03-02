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
 *  this function calculates the median of the image data       
*/

/** @brief calculates the median of the image data         
 *
 *  @param pic_old    pointer to the image 
 *
 * @return  median of the image data       
 *
 * AUTHOR & DATE
 */

/* include files                                                        */

#include "mitkIpFuncP.h"   

mitkIpFloat8_t mitkIpFuncMedI ( mitkIpPicDescriptor *pic_old );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncMedI\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* -------------------------------------------------------------------  */
/*
**  function mitkIpFuncMedI:                                             
*/
/* -------------------------------------------------------------------  */

mitkIpFloat8_t mitkIpFuncMedI ( mitkIpPicDescriptor *pic_old )
{

  mitkIpFloat8_t      max_gv;    /* max. possible greyvalue                 */
  mitkIpFloat8_t      min_gv;    /* min. possible greyvalue                 */
  mitkIpUInt4_t       *hist;     /* greylevel histogram                     */
  mitkIpUInt4_t       size_hist; /* no. of elements in histogram            */
  mitkIpUInt4_t       i;         /* loop index                              */
  mitkIpUInt4_t       sum;       /* sum of histogram elements               */
  mitkIpUInt4_t       limit;     /*                                         */
  mitkIpFloat4_t      factor;  
  mitkIpFloat8_t      median;    /* median of image data                    */

  /* check whether image data are ok                                    */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues                        */

  if ( _mitkIpFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) == mitkIpFuncERROR ) 
    {
       return ( mitkIpFuncERROR );
    }

  /* calculate greylevel histogram                                      */

  mitkIpFuncHist ( pic_old, min_gv, max_gv, &hist, &size_hist ); 
  if ( hist == 0 ) 
    {
       return ( mitkIpFuncERROR );
    }

  /* factor to calculate the greyvalue belonging to an histogram index  */
 
  if ( pic_old->type == mitkIpPicFloat ) 
    factor = 0.001;                              
  else if ( pic_old->type == mitkIpPicInt || pic_old->type == mitkIpPicUInt )
    factor = 1.;
  else 
    {
       free ( hist );
       _mitkIpFuncSetErrno ( mitkIpFuncTYPE_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* find median                                                        */

  limit = _mitkIpPicElements ( pic_old ) / 2;
  for ( i = 0, sum = 0; sum < limit; i++ )
    sum = sum + hist [i];

  median = i * factor - fabs ( min_gv );

  free ( hist );

  return ( median );
}
#endif

