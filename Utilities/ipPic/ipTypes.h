
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   defines the ipTypes
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) Heidelberg, FRG
 */

#ifndef _ipTypes_h
#define _ipTypes_h

#if defined (__mips) || (_IBMR2) || (sun) || (__i386__) || (__convex__) || (__alpha) || (__hppa) || (WIN32)
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

#endif  /* _ipTypes_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
