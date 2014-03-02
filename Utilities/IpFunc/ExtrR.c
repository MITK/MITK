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
 *   this function calculates the extreme values of all greyvalues included by a rectangle
 */
  

/**  @brief calculates the extreme values of all greyvalues included by a rectangle
 *
 *   The rectangle is described by a vector with the coordinates of the 
 *   upper left corner and a vector of its length in each direction
 *
 *   @param pic_old        pointer to original image
 *   @param min     pointer to minimal greyvalue ( return value )
 *   @param max       pointer to maximal greyvalue ( return value )
 *   @param begin      vector with the beginning coordinates of the window
 *   @param length        vector with the length of the window in each direction
 *
 *   @return mitkIpFuncOK           if no error occured
 *   @return mitkIpFuncERROR       if an error occured
 *
 * AUTHOR & DATE
 */
#include "mitkIpFuncP.h"

mitkIpUInt4_t  mitkIpFuncExtrR  ( mitkIpPicDescriptor *pic_old,
                          mitkIpFloat8_t      *min,   
                          mitkIpFloat8_t      *max, 
                          mitkIpUInt4_t       *begin,
                          mitkIpUInt4_t       *length );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncExtrR\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


mitkIpUInt4_t  mitkIpFuncExtrR  ( mitkIpPicDescriptor *pic_old,
                          mitkIpFloat8_t      *min,   
                          mitkIpFloat8_t      *max, 
                          mitkIpUInt4_t       *begin,
                          mitkIpUInt4_t       *length )
{

   mitkIpPicDescriptor *pic_help;
   mitkIpInt4_t        error;

   pic_help = mitkIpFuncWindow ( pic_old, begin, length );

   if ( pic_help )
      error = mitkIpFuncExtr ( pic_help, min, max );
   else 
     return ( mitkIpFuncERROR );

   mitkIpPicFree ( pic_help );

   return ( error );
}
#endif
