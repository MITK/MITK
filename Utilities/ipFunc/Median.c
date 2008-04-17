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

#include "ipFuncP.h"   

ipFloat8_t ipFuncMedI ( ipPicDescriptor *pic_old );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncMedI\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* -------------------------------------------------------------------  */
/*
**  function ipFuncMedI:                                             
*/
/* -------------------------------------------------------------------  */

ipFloat8_t ipFuncMedI ( ipPicDescriptor *pic_old )
{

  ipFloat8_t      max_gv;    /* max. possible greyvalue                 */
  ipFloat8_t      min_gv;    /* min. possible greyvalue                 */
  ipUInt4_t       *hist;     /* greylevel histogram                     */
  ipUInt4_t       size_hist; /* no. of elements in histogram            */
  ipUInt4_t       i;         /* loop index                              */
  ipUInt4_t       sum;       /* sum of histogram elements               */
  ipUInt4_t       limit;     /*                                         */
  ipFloat4_t      factor;  
  ipFloat8_t      median;    /* median of image data                    */

  /* check whether image data are ok                                    */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues                        */

  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) == mitkIpFuncERROR ) 
    {
       return ( mitkIpFuncERROR );
    }

  /* calculate greylevel histogram                                      */

  ipFuncHist ( pic_old, min_gv, max_gv, &hist, &size_hist ); 
  if ( hist == 0 ) 
    {
       return ( mitkIpFuncERROR );
    }

  /* factor to calculate the greyvalue belonging to an histogram index  */
 
  if ( pic_old->type == ipPicFloat ) 
    factor = 0.001;                              
  else if ( pic_old->type == ipPicInt || pic_old->type == ipPicUInt )
    factor = 1.;
  else 
    {
       free ( hist );
       _ipFuncSetErrno ( mitkIpFuncTYPE_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* find median                                                        */

  limit = _ipPicElements ( pic_old ) / 2;
  for ( i = 0, sum = 0; sum < limit; i++ )
    sum = sum + hist [i];

  median = i * factor - fabs ( min_gv );

  free ( hist );

  return ( median );
}
#endif

