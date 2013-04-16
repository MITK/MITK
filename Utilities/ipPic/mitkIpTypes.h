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


/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   defines the ipTypes
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) Heidelberg, FRG
 */

#ifndef _mitkIpTypes_h
#define _mitkIpTypes_h

#if defined (__mips) || (_IBMR2) || (sun) || (__i386__) || (__convex__) || (__alpha) || (__hppa) || (WIN32) || (_WIN32) || defined (__PPC__) || defined (__x86_64__) || defined (__APPLE__)
  typedef    signed char  mitkIpInt1_t;
  typedef   signed short  mitkIpInt2_t;
  typedef     signed int  mitkIpInt4_t;
  typedef  unsigned char  mitkIpUInt1_t;
  typedef unsigned short  mitkIpUInt2_t;
  typedef   unsigned int  mitkIpUInt4_t;
  typedef          float  mitkIpFloat4_t;
  typedef         double  mitkIpFloat8_t;
#endif

#ifdef DOS
  typedef       signed char  mitkIpInt1_t;
  typedef        signed int  mitkIpInt2_t;
  typedef   signed long int  mitkIpInt4_t;
  typedef     unsigned char  mitkIpUInt1_t;
  typedef      unsigned int  mitkIpUInt2_t;
  typedef unsigned long int  mitkIpUInt4_t;
  typedef             float  mitkIpFloat4_t;
  typedef            double  mitkIpFloat8_t;
#endif

/*
** this boolean is NOT a 1 bit value !!!
** it is NOT intended for 1 bit images !!!
*/
typedef enum
  { mitkIpFalse = 0,
    mitkIpTrue  = 1
  } mitkIpBool_t;

typedef enum
  {
    mitkIpError = -1,            /* general error              */
    mitkIpOK    =  0,            /* no error                   */
    mitkIpEDIM  =  1000          /* wrong number of dimensions */
  } mitkIpError_t;


typedef enum
  {
    _mitkIpEndianUnknown = 0,
    _mitkIpEndianLittle  = 1,
    _mitkIpEndianBig     = 2
  } _mitkIpEndian_t;

typedef enum
  {
    mitkIpTypeUnknown = 0,
    mitkIpTypeStringPtr,
    mitkIpTypeString,
    mitkIpTypeInt1,
    mitkIpTypeInt2,
    mitkIpTypeInt4,
    mitkIpTypeUInt1,
    mitkIpTypeUInt2,
    mitkIpTypeUInt4,
    mitkIpTypeFloat4,
    mitkIpTypeFloat8,
    mitkIpTypeNBytes,
    mitkIpTypeArray,
    mitkIpTypeList,
    mitkIpTypePic,
    mitkIpTypePicTag,
    _mitkIpTypeMAX
  } mitkIpType_t;

#endif  /* _mitkIpTypes_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
