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
 *  Point-Operators.
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

mitkIpPicDescriptor *mitkIpFuncInv    ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncEqual  ( mitkIpPicDescriptor *pic_old,   
                                mitkIpFuncFlagI_t   kind,
                                mitkIpPicDescriptor *pic_return );


mitkIpPicDescriptor *mitkIpFuncPot    ( mitkIpPicDescriptor *pic_1,
                                ipFloat8_t      exponent,
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncSqrt   ( mitkIpPicDescriptor *pic_1,
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncThresh ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t       threshold,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncZeroCr ( mitkIpPicDescriptor *pic_old );

mitkIpPicDescriptor *mitkIpFuncExp    ( mitkIpPicDescriptor *pic_old,
                                mitkIpFuncFlagI_t   kind,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncLN     ( mitkIpPicDescriptor *pic_old );

mitkIpPicDescriptor *mitkIpFuncLog    ( mitkIpPicDescriptor *pic_old );

mitkIpPicDescriptor *mitkIpFuncNorm   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncNormXY ( mitkIpPicDescriptor *pic, 
                                ipFloat8_t min_gv,
                                ipFloat8_t max_gv,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncSelInv ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up, 
                                ipFloat8_t      gv,
                                mitkIpPicDescriptor *pic_return );           

mitkIpPicDescriptor *mitkIpFuncSelect ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up, 
                                ipFloat8_t      gv,
                                mitkIpPicDescriptor *pic_return );           

mitkIpPicDescriptor *mitkIpFuncSelMM  ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncLevWin ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      level,   
                                ipFloat8_t      window,
                                mitkIpPicDescriptor *pic_return );           
      
void            mitkIpFuncXchange   ( mitkIpPicDescriptor **pic1,
                                  mitkIpPicDescriptor **pic2 );

mitkIpPicDescriptor *mitkIpFuncConvert( mitkIpPicDescriptor *pic_old,
                                mitkIpPicType_t     type,
                                ipUInt4_t       bpe );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif /* _mitkIpFunc_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */

