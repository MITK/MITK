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
 *  this function returns the allowed greyvalue range for a specific data-type
 */

/** @brief this function returns the allowed greyvalue range for a 
 *  specific data-type
 *
 *  @param type     data-type of image data
 *  @param bpe     number of bits per element
 *  @param max_gv  max. possible greyvalue
 *  @param min_gv   min. possible greyvalue
 *
 *  @return mitkIpFuncERROR  if an error occured
 *  @return mitkIpFuncOK   when no error occured 
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   

ipInt4_t _mitkIpFuncExtT ( mitkIpPicType_t type,
                      ipUInt4_t   bpe,
                      ipFloat8_t  *min_gv,
                      ipFloat8_t  *max_gv );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)_mitkIpFuncExtT\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif




#ifdef _WIN32
#include <float.h>
#endif
 
/* definition of EXP-macro                                              */

#define EXP( max_gv, bpe )                                               \
{                                                                        \
  ipUInt4_t i;                                                           \
                                                                         \
  *max_gv = 1;                                                           \
  for ( i = 1; i <= bpe; i++ )                                           \
     *max_gv = *max_gv * 2;                                              \
} 

/* -------------------------------------------------------------------  */
/*
**  function _picExtT: calculates the max. and min. possible greyvalues for
**                    a certain datatype                              
*/
/* -------------------------------------------------------------------  */

ipInt4_t _mitkIpFuncExtT ( mitkIpPicType_t type,
                      ipUInt4_t   bpe,
                      ipFloat8_t  *min_gv,
                      ipFloat8_t  *max_gv )
{

  /* calculate max. or min possible greyvalue for datatype              */

  if ( type == mitkIpPicInt )
    {
       EXP( max_gv, bpe - 1 )
       *max_gv = *max_gv - 1;
       *min_gv = - *max_gv;
    }
  else if ( type == mitkIpPicUInt )
    {
       EXP (max_gv, bpe )
       *max_gv = *max_gv - 1;
       *min_gv = 0;
    }
  else if ( type == mitkIpPicFloat )
    {
       if ( bpe = 32 )
         {
            *max_gv = FLT_MAX;     
            *min_gv = - FLT_MAX;  
         }
       else if ( bpe = 64 )
         {
            *max_gv = DBL_MAX;
            *min_gv = - DBL_MAX;
         }
    }
  else 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncTYPE_ERROR );
       return ( mitkIpFuncERROR );
    }

  return ( mitkIpFuncOK );
}
#endif
