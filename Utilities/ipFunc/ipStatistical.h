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
 *  Operators to calculate statistical values.
 *  This header is for grouping purposes for the
 *  documentation only. Don't use it, use mitkIpFunc.h instead.
 */

#ifndef DOXYGEN_IGNORE
#include <mitkIpFunc.h>
#endif

#ifndef _mitkIpFunc_h
#define _mitkIpFunc_h

/* function prototypes */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

mitkIpUInt4_t mitkIpFuncBox ( mitkIpPicDescriptor *pic_old,
                      mitkIpUInt4_t       **beg,
                      mitkIpUInt4_t       **end );

mitkIpFloat8_t      mitkIpFuncMedI    ( mitkIpPicDescriptor *pic_old );

mitkIpFloat8_t      mitkIpFuncMean    ( mitkIpPicDescriptor *pic_old );

mitkIpFloat8_t      mitkIpFuncVar     ( mitkIpPicDescriptor *pic_old );

mitkIpFloat8_t      mitkIpFuncSDev    ( mitkIpPicDescriptor *pic_old );

mitkIpFloat8_t      *mitkIpFuncGrav   ( mitkIpPicDescriptor *pic_old );

mitkIpFloat8_t       mitkIpFuncMeanC  ( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       radius  );

mitkIpFloat8_t       mitkIpFuncVarC   ( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       radius  );

mitkIpFloat8_t       mitkIpFuncSDevC  ( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       radius  );

mitkIpUInt4_t        mitkIpFuncExtrC  ( mitkIpPicDescriptor *pic_old,        
                                mitkIpFloat8_t      *min,
                                mitkIpFloat8_t      *max,  
                                mitkIpUInt4_t       *center,  
                                mitkIpUInt4_t       radius );

mitkIpFloat8_t       mitkIpFuncMeanROI( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *pol_x,   
                                mitkIpUInt4_t       *pol_y,    
                                mitkIpUInt4_t       no_pts );

mitkIpFloat8_t       mitkIpFuncVarROI( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *pol_x,   
                                mitkIpUInt4_t       *pol_y,    
                                mitkIpUInt4_t       no_pts );

mitkIpFloat8_t       mitkIpFuncSDevROI( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *pol_x,   
                                mitkIpUInt4_t       *pol_y,    
                                mitkIpUInt4_t       no_pts );

mitkIpFloat8_t      mitkIpFuncSkewness ( mitkIpPicDescriptor *pic );

mitkIpFloat8_t      mitkIpFuncCurtosis ( mitkIpPicDescriptor *pic );

mitkIpFloat8_t      mitkIpFuncSkewnessR ( mitkIpPicDescriptor *pic_old,
                                  mitkIpUInt4_t       *begin, 
                                  mitkIpUInt4_t       *length );

double          mitkIpFuncCurtosisR ( mitkIpPicDescriptor *pic_old,
                                  mitkIpUInt4_t       *begin, 
                                  mitkIpUInt4_t       *length );

mitkIpUInt4_t        mitkIpFuncExtrROI( mitkIpPicDescriptor *pic_old,        
                                mitkIpFloat8_t      *min,
                                mitkIpFloat8_t      *max, 
                                mitkIpUInt4_t       *pol_x,   
                                mitkIpUInt4_t       *pol_y,    
                                mitkIpUInt4_t       no_pts );

mitkIpFloat8_t       mitkIpFuncMeanR  ( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       *length );

mitkIpFloat8_t       mitkIpFuncVarR   ( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       *length );

mitkIpFloat8_t       mitkIpFuncSDevR  ( mitkIpPicDescriptor *pic_old,        
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       *length );

mitkIpUInt4_t        mitkIpFuncExtrR  ( mitkIpPicDescriptor *pic_old,        
                                mitkIpFloat8_t      *min,
                                mitkIpFloat8_t      *max,  
                                mitkIpUInt4_t       *begin,   
                                mitkIpUInt4_t       *length );

mitkIpUInt4_t       mitkIpFuncRange   ( mitkIpPicDescriptor *pic, 
                                mitkIpFloat8_t gv_low,
                                mitkIpFloat8_t gv_up );

mitkIpInt4_t        mitkIpFuncExtr    ( mitkIpPicDescriptor *pic, 
                                mitkIpFloat8_t *min,
                                mitkIpFloat8_t *max );

mitkIpInt4_t       mitkIpFuncInertia ( mitkIpPicDescriptor *pic_old,   
                               mitkIpFloat8_t      **eigen_vect,
                               mitkIpFloat8_t      **eigen_val );

mitkIpInt4_t       mitkIpFuncHist   ( mitkIpPicDescriptor *pic_old,   
                              mitkIpFloat8_t      min_gv,  
                              mitkIpFloat8_t      max_gv,
                              mitkIpUInt4_t       **hist,
                              mitkIpUInt4_t       *size_hist );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif /* _mitkIpFunc_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */

