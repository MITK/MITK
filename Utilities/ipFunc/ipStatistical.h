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

ipUInt4_t mitkIpFuncBox ( mitkIpPicDescriptor *pic_old,
                      ipUInt4_t       **beg,
                      ipUInt4_t       **end );

ipFloat8_t      mitkIpFuncMedI    ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      mitkIpFuncMean    ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      mitkIpFuncVar     ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      mitkIpFuncSDev    ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      *mitkIpFuncGrav   ( mitkIpPicDescriptor *pic_old );

ipFloat8_t       mitkIpFuncMeanC  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       mitkIpFuncVarC   ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       mitkIpFuncSDevC  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipUInt4_t        mitkIpFuncExtrC  ( mitkIpPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *center,  
                                ipUInt4_t       radius );

ipFloat8_t       mitkIpFuncMeanROI( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncVarROI( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncSDevROI( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t      mitkIpFuncSkewness ( mitkIpPicDescriptor *pic );

ipFloat8_t      mitkIpFuncCurtosis ( mitkIpPicDescriptor *pic );

ipFloat8_t      mitkIpFuncSkewnessR ( mitkIpPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

double          mitkIpFuncCurtosisR ( mitkIpPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

ipUInt4_t        mitkIpFuncExtrROI( mitkIpPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max, 
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncMeanR  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       mitkIpFuncVarR   ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       mitkIpFuncSDevR  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipUInt4_t        mitkIpFuncExtrR  ( mitkIpPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipUInt4_t       mitkIpFuncRange   ( mitkIpPicDescriptor *pic, 
                                ipFloat8_t gv_low,
                                ipFloat8_t gv_up );

ipInt4_t        mitkIpFuncExtr    ( mitkIpPicDescriptor *pic, 
                                ipFloat8_t *min,
                                ipFloat8_t *max );

ipInt4_t       mitkIpFuncInertia ( mitkIpPicDescriptor *pic_old,   
                               ipFloat8_t      **eigen_vect,
                               ipFloat8_t      **eigen_val );

ipInt4_t       mitkIpFuncHist   ( mitkIpPicDescriptor *pic_old,   
                              ipFloat8_t      min_gv,  
                              ipFloat8_t      max_gv,
                              ipUInt4_t       **hist,
                              ipUInt4_t       *size_hist );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif /* _mitkIpFunc_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */

