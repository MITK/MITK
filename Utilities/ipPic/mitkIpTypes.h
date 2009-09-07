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
  typedef    signed char  ipInt1_t;
  typedef   signed short  ipInt2_t;
  typedef     signed int  ipInt4_t;
  typedef  unsigned char  ipUInt1_t;
  typedef unsigned short  ipUInt2_t;
  typedef   unsigned int  ipUInt4_t;
  typedef          float  ipFloat4_t;
  typedef         double  ipFloat8_t;
#endif

#ifdef DOS
  typedef       signed char  ipInt1_t;
  typedef        signed int  ipInt2_t;
  typedef   signed long int  ipInt4_t;
  typedef     unsigned char  ipUInt1_t;
  typedef      unsigned int  ipUInt2_t;
  typedef unsigned long int  ipUInt4_t;
  typedef             float  ipFloat4_t;
  typedef            double  ipFloat8_t;
#endif

/*
** this boolean is NOT a 1 bit value !!!
** it is NOT intended for 1 bit images !!!
*/
typedef enum
  { ipFalse = 0,
    ipTrue  = 1
  } ipBool_t;

typedef enum
  {
    ipError = -1,            /* general error              */
    ipOK    =  0,            /* no error                   */
    ipEDIM  =  1000          /* wrong number of dimensions */
  } ipError_t;


typedef enum
  {
    _ipEndianUnknown = 0,
    _ipEndianLittle  = 1,
    _ipEndianBig     = 2
  } _ipEndian_t;

typedef enum
  {
    ipTypeUnknown = 0,
    ipTypeStringPtr,
    ipTypeString,
    ipTypeInt1,
    ipTypeInt2,
    ipTypeInt4,
    ipTypeUInt1,
    ipTypeUInt2,
    ipTypeUInt4,
    ipTypeFloat4,
    ipTypeFloat8,
    ipTypeNBytes,
    ipTypeArray,
    ipTypeList,
    ipTypePic,
    ipTypePicTag,
    _ipTypeMAX
  } ipType_t;

#endif  /* _mitkIpTypes_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
