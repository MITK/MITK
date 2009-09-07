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
 *    this function calculates the curtosis of all 
 *    greyvalues in an image
 */

/** @brief calculates the curtosis of all 
 *    greyvalues in an image
 *
 *   @param pic          pointer to the image 
 *  
 *   @return  curtosis of image
 *
 * @author Steffen Gundel
 */
 
 
/* include-files                                              */

#include "mitkIpFuncP.h"

ipFloat8_t mitkIpFuncCurtosis ( mitkIpPicDescriptor *pic );

/* definition of extreme value macro                          */

#ifndef DOXYGEN_IGNORE

/* include-files                                              */

#include "mitkIpFuncP.h"
 
/* definition of extreme value macro                          */

#define CURTOSIS( type, pic, mean, excess )                      \
  {                                                            \
    ipUInt4_t   i, no_elem;                                    \
                                                               \
    excess = 0.;                                               \
                                                               \
    no_elem = _mitkIpPicElements ( pic );                          \
    for ( i = 0; i < no_elem; i++ )                            \
      {                                                        \
        excess = ( ( ( type * ) pic->data ) [i] - mean ) *     \
                 ( ( ( type * ) pic->data ) [i] - mean ) *     \
				 ( ( ( type * ) pic->data ) [i] - mean ) *     \
				 ( ( ( type * ) pic->data ) [i] - mean ) +     \
                 excess;                                       \
      }                                                        \
  }
 
ipFloat8_t mitkIpFuncCurtosis ( mitkIpPicDescriptor *pic )
{

  ipFloat8_t curt;
  ipFloat8_t mean, std;            
             

  /* check image data                                         */

  if ( _mitkIpFuncError ( pic ) != ipOK ) return ( mitkIpFuncERROR );

  curt = 0.0;
  if (  _mitkIpPicElements ( pic ) != 1 ) 
    {
       
       mean =  mitkIpFuncMean ( pic );
	   std = mitkIpFuncSDev( pic );

       mitkIpPicFORALL_2( CURTOSIS, pic, mean,  curt );   
  
       curt =   curt / ((( ipFloat8_t )(_mitkIpPicElements ( pic ) - 1 )) *std*std*std*std) -3;
    }

  return( curt );
}
#endif

