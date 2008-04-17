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
 *  documentation only. Don't use it, use ipFunc.h instead.
 */

#ifndef DOXYGEN_IGNORE
#include <ipFunc.h>
#endif

#ifndef _mitkIpFunc_h
#define _mitkIpFunc_h

/* function prototypes */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

ipUInt4_t ipFuncBox ( ipPicDescriptor *pic_old,
                      ipUInt4_t       **beg,
                      ipUInt4_t       **end );

ipFloat8_t      ipFuncMedI    ( ipPicDescriptor *pic_old );

ipFloat8_t      ipFuncMean    ( ipPicDescriptor *pic_old );

ipFloat8_t      ipFuncVar     ( ipPicDescriptor *pic_old );

ipFloat8_t      ipFuncSDev    ( ipPicDescriptor *pic_old );

ipFloat8_t      *ipFuncGrav   ( ipPicDescriptor *pic_old );

ipFloat8_t       ipFuncMeanC  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       ipFuncVarC   ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       ipFuncSDevC  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipUInt4_t        ipFuncExtrC  ( ipPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *center,  
                                ipUInt4_t       radius );

ipFloat8_t       ipFuncMeanROI( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncVarROI( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncSDevROI( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t      ipFuncSkewness ( ipPicDescriptor *pic );

ipFloat8_t      ipFuncCurtosis ( ipPicDescriptor *pic );

ipFloat8_t      ipFuncSkewnessR ( ipPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

double          ipFuncCurtosisR ( ipPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

ipUInt4_t        ipFuncExtrROI( ipPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max, 
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncMeanR  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       ipFuncVarR   ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       ipFuncSDevR  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipUInt4_t        ipFuncExtrR  ( ipPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipUInt4_t       ipFuncRange   ( ipPicDescriptor *pic, 
                                ipFloat8_t gv_low,
                                ipFloat8_t gv_up );

ipInt4_t        ipFuncExtr    ( ipPicDescriptor *pic, 
                                ipFloat8_t *min,
                                ipFloat8_t *max );

ipInt4_t       ipFuncInertia ( ipPicDescriptor *pic_old,   
                               ipFloat8_t      **eigen_vect,
                               ipFloat8_t      **eigen_val );

ipInt4_t       ipFuncHist   ( ipPicDescriptor *pic_old,   
                              ipFloat8_t      min_gv,  
                              ipFloat8_t      max_gv,
                              ipUInt4_t       **hist,
                              ipUInt4_t       *size_hist );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif /* _mitkIpFunc_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */

