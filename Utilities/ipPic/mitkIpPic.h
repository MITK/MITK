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
  */

/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   public defines for the pic-format
 *
 * $Log$
 * Revision 1.3  2007/03/22 13:41:29  nolden
 * FIX: resolve problems with chili plugin
 *
 * Revision 1.2  2007/03/07 17:23:18  nolden
 * fixes for chili plugin build
 *
 * Revision 1.1  2006/12/13 11:54:43  nolden
 * *** empty log message ***
 *
 * Revision 1.47  2006/11/20 16:09:18  nolden
 * FIX/CHG: different mangling of zlib names for ITK 3.x
 *
 * Revision 1.46  2006/09/20 11:38:17  nolden
 * CHG: use itk zlib header from itkzlib subdir of ipPic if USE_ITKZLIB is defined
 *
 * Revision 1.45  2006/04/20 12:56:00  nolden
 * FIX/CHG/ENH: added PPC-Mac definitions
 *
 * Revision 1.44  2005/10/11 19:06:41  ivo
 * FIX: typo
 *
 * Revision 1.43  2005/10/11 18:35:34  ivo
 * FIX: typo
 *
 * Revision 1.42  2005/10/11 18:19:46  ivo
 * ENH: trying to fix zlib problems: use renamed itk zlib methods (itk_gzread etc)
 *
 * Revision 1.41  2005/04/29 15:41:46  ivo
 * FIX: qchili compatibility for version of 21/4/05
 *
 * Revision 1.40  2004/04/06 14:59:39  nolden
 * moved win32 compatibility settings from mitkIpPic.h to ipPic.pro
 *
 * Revision 1.39  2004/03/25 18:35:05  andre
 * bug fix
 *
 * Revision 1.38  2003/12/02 11:54:31  tobiask
 * Added setters and getters for rgb pixels.
 *
 * Revision 1.37  2003/11/28 14:24:45  tobiask
 * Fixed bugs in macros mitkIpPicGET_PIXEL and mitkIpPicPUT_PIXEL.
 *
 * Revision 1.36  2003/09/30 20:12:50  tobiask
 * Added macros for getting and setting pixel values.
 *
 * Revision 1.35  2003/02/18 12:28:23  andre
 * write compressed pic files
 *
 * Revision 1.34  2002/11/13 17:52:59  ivo
 * new ipPic added.
 *
 * Revision 1.32  2002/10/31 10:00:30  ivo
 * zlib.lib including headers now in win32. gz-extension should work now on every windows.
 * WIN32 define changed to standard _WIN32.
 *
 * Revision 1.31  2002/05/16 13:14:30  peter
 * changed include path to ipPic/mitkIpPic.h
 *
 * Revision 1.30  2002/04/18 19:09:13  ivo
 * compression (zlib) activated.
 *
 * Revision 1.1.1.1  2002/04/18 18:38:43  ivo
 * no message
 *
 * Revision 1.29  2002/02/27 08:54:43  andre
 * zlib changes
 *
 * Revision 1.28  2000/06/26 16:02:05  andre
 * pPicFORALL_x bug fix
 *
 * Revision 1.27  2000/05/24 14:10:51  tobiask
 * Removed ipPic/ from the include directives.
 *
 * Revision 1.26  2000/05/04 12:52:35  ivo
 * inserted BSD style license
 *
 * Revision 1.25  2000/05/04 12:35:58  ivo
 * some doxygen comments.
 *
 * Revision 1.24  1999/12/09  19:17:46  andre
 * *** empty log message ***
 *
 * Revision 1.23  1999/12/09  19:11:51  andre
 * *** empty log message ***
 *
 * Revision 1.22  1999/11/29  09:41:39  andre
 * *** empty log message ***
 *
 * Revision 1.21  1999/11/29  09:34:34  andre
 * *** empty log message ***
 *
 * Revision 1.20  1999/11/28  16:29:43  andre
 * *** empty log message ***
 *
 * Revision 1.19  1999/11/28  16:27:21  andre
 * *** empty log message ***
 *
 * Revision 1.18  1999/11/27  23:59:45  andre
 * *** empty log message ***
 *
 * Revision 1.17  1999/11/27  19:15:08  andre
 * *** empty log message ***
 *
 * Revision 1.16  1999/11/27  12:47:52  andre
 * *** empty log message ***
 *
 * Revision 1.15  1999/08/20  08:25:42  andre
 * *** empty log message ***
 *
 * Revision 1.14  1998/09/16  18:35:16  andre
 * *** empty log message ***
 *
 * Revision 1.13  1998/09/16  18:32:53  andre
 * *** empty log message ***
 *
 * Revision 1.12  1998/07/16  12:33:18  andre
 * *** empty log message ***
 *
 * Revision 1.11  1998/07/14  11:16:16  andre
 * *** empty log message ***
 *
 * Revision 1.10  1998/06/23  15:56:40  andre
 * *** empty log message ***
 *
 * Revision 1.9  1998/06/18  09:12:14  andre
 * *** empty log message ***
 *
 * Revision 1.8  1998/06/09  12:38:30  andre
 * added mitkIpPicDelSubTag
 *
 * Revision 1.7  1998/06/09  11:34:32  andre
 * *** empty log message ***
 *
 * Revision 1.6  1998/06/09  10:58:25  andre
 * added mitkIpPicAddSubTag
 *
 * Revision 1.5  1998/05/09  18:30:23  andre
 * *** empty log message ***
 *
 * Revision 1.4  1998/05/06  14:13:15  andre
 * added info->pixel_start_in_file
 *
 * Revision 1.3  1997/10/20  13:35:40  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  10:24:12  andre
 * mitkIpPicPut now returns status
 *
 * Revision 0.2  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.1  1993/03/31  11:44:35  andre
 * added 'GetSlice'
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */

#define USE_ITKZLIB

#ifndef _mitkIpPic_h
#define _mitkIpPic_h


#ifndef NO_DEBUG
#  define mitkIpPicDEBUG
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef sun
#  include <unistd.h>
#endif



#ifdef WIN
#  include <windows.h>
extern DWORD ipWinFread(LPSTR ptr, DWORD size, DWORD n, FILE *stream);
#endif



#include <assert.h>     /**/

#include <ipPic/mitkIpTypes.h>


#ifdef mitkIpPicDEBUG
#endif


/*
** defines
*/
#define mitkIpPicVERSION "PIC VERSION 3.00                "
#define mitkIpPicVERSIONe "PIC VERSION 3.00e               "
#define _mitkIpPicNDIM 8
#define _mitkIpPicTAGLEN 32


#if (THIS_IS_BIG_ENDIAN) || defined (_IBMR2) || (mc68000) || (sun) || (__convex__) || (__hppa) || (_MIPSEB) || defined ( __PPC__)
#  define _mitkIpENDIAN_IS_BIG
#else
#  define _mitkIpENDIAN_IS_LITTLE
#endif


#define mitkIpPicVersionMajor(pic) (mitkIpUInt1_t)(pic->info->version[12] - '0')
#define mitkIpPicVersionMinor(pic) (mitkIpUInt1_t)(10 * (pic->info->version[14]-'0' ) \
                                           + pic->info->version[15]-'0')
#define mitkIpPicEncryptionType(pic) (pic->info->version[16])

/*#define mitkIpPicIsPic(pic) (( (pic)->magic == mitkIpPicPicDescriptor_m) \
                             ? mitkIpTrue : mitkIpFalse )*/


#define mitkIpPicDR(type,len) (mitkIpUInt4_t)( ((type)<<16) + (len) )

#define mitkIpPicFOR_TYPE( _ipPIC_TYPE, _ipPIC_CMD, _ipPIC_PIC) \
  _ipPIC_CMD(_ipPIC_TYPE,_ipPIC_PIC)

#define mitkIpPicFORALL( _ipPIC_CMD, _ipPIC_PIC )                                    \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC);                                        \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC);                                       \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC);                                        \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC);                                       \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC);                                        \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC);                                       \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC);                                      \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC);                                      \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define mitkIpPicFORALL_1( _ipPIC_CMD, _ipPIC_PIC, A1 )                              \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC, A1);                                    \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC, A1);                                   \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC, A1);                                    \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC, A1);                                   \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC, A1);                                    \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC, A1);                                   \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC, A1);                                  \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC, A1);                                  \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define mitkIpPicFORALL_2( _ipPIC_CMD, _ipPIC_PIC, A1, A2 )                          \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC, A1, A2);                                \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC, A1, A2);                               \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC, A1, A2);                                \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC, A1, A2);                               \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC, A1, A2);                                \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC, A1, A2);                               \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC, A1, A2);                              \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC, A1, A2);                              \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define mitkIpPicFORALL_3( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3 )                      \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC, A1, A2, A3);                            \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC, A1, A2, A3);                           \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC, A1, A2, A3);                            \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC, A1, A2, A3);                           \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC, A1, A2, A3);                            \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC, A1, A2, A3);                           \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC, A1, A2, A3);                          \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC, A1, A2, A3);                          \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define mitkIpPicFORALL_4( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3, A4 )                  \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC, A1, A2, A3, A4);                        \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC, A1, A2, A3, A4);                       \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC, A1, A2, A3, A4);                        \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC, A1, A2, A3, A4);                       \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC, A1, A2, A3, A4);                        \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC, A1, A2, A3, A4);                       \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC, A1, A2, A3, A4);                      \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC, A1, A2, A3, A4);                      \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define mitkIpPicFORALL_5( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3, A4, A5 )              \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                    \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                   \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                    \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                   \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                    \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                   \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                  \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                  \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define mitkIpPicFORALL_6( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3, A4, A5, A6 )          \
  switch( mitkIpPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case mitkIpPicDR( mitkIpPicInt, 8 ):                                               \
        _ipPIC_CMD(mitkIpInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);                \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 8 ):                                              \
        _ipPIC_CMD(mitkIpUInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);               \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 16 ):                                              \
        _ipPIC_CMD(mitkIpInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);                \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 16 ):                                             \
        _ipPIC_CMD(mitkIpUInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);               \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicInt, 32 ):                                              \
        _ipPIC_CMD(mitkIpInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);                \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicUInt, 32 ):                                             \
        _ipPIC_CMD(mitkIpUInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);               \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 32 ):                                            \
        _ipPIC_CMD(mitkIpFloat4_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);              \
        break;                                                                   \
      case mitkIpPicDR( mitkIpPicFloat, 64 ):                                            \
        _ipPIC_CMD(mitkIpFloat8_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);              \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define _mitkIpPicGET_PIXEL(TYPE,PIC,X,Y,RESULT) \
{                                            \
  TYPE* data = (TYPE *) (PIC)->data          \
  + (X) + (Y) * (PIC)->n[0];                 \
  RESULT = *data;                            \
}

#define _mitkIpPicPUT_PIXEL(TYPE,PIC,X,Y,VALUE)  \
{                                            \
  TYPE* data = (TYPE *) (PIC)->data          \
  + (X) + (Y) * (PIC)->n[0];                 \
  *data = VALUE;                             \
}

#define mitkIpPicGET_PIXEL(PIC,X,Y,RESULT)             \
{                                                  \
  mitkIpPicFORALL_3(_mitkIpPicGET_PIXEL,PIC,X,Y,RESULT);   \
}

#define mitkIpPicPUT_PIXEL(PIC,X,Y,RESULT)             \
{                                                  \
  mitkIpPicFORALL_3(_mitkIpPicPUT_PIXEL,PIC,X,Y,RESULT);   \
}

#define _mitkIpPicGET_PIXEL_RGB(TYPE,PIC,X,Y,R,G,B)                \
{                                                              \
  mitkIpUInt4_t skip = (PIC)->n[0] * (PIC)->n[1];                  \
  TYPE* data = (TYPE *) (PIC)->data + (X) + (Y) * (PIC)->n[0]; \
  R = *(data);                                                 \
  G = *(data + skip);                                          \
  B = *(data + skip + skip);                                   \
}

#define _mitkIpPicPUT_PIXEL_RGB(TYPE,PIC,X,Y,R,G,B)                \
{                                                              \
  mitkIpUInt4_t skip = (PIC)->n[0] * (PIC)->n[1];                  \
  TYPE* data = (TYPE *) (PIC)->data + (X) + (Y) * (PIC)->n[0]; \
  *data = (R);                                                 \
  *(data + skip) = (G);                                        \
  *(data + skip + skip) = (B);                                 \
}

#define mitkIpPicGET_PIXEL_RGB(PIC,X,Y,R,G,B)             \
{                                                     \
  mitkIpPicFORALL_5(_mitkIpPicGET_PIXEL_RGB,PIC,X,Y,R,G,B);   \
}

#define mitkIpPicPUT_PIXEL_RGB(PIC,X,Y,R,G,B)             \
{                                                     \
  mitkIpPicFORALL_5(_mitkIpPicPUT_PIXEL_RGB,PIC,X,Y,R,G,B);   \
}

#define USE_ZLIB /**/
#ifdef USE_ZLIB
#ifdef USE_ITKZLIB
#  define mitkIpPicFile_t gzFile
#  define mitkIpPicFOpen gzopen
#  define mitkIpPicFTell gztell
#  define mitkIpPicFSeek gzseek
#  define mitkIpPicFRead(buf, bsize, count, file) gzread((file),(buf),((count)*(bsize)))
#  define mitkIpPicFWrite _mitkIpPicFWrite
#  define mitkIpPicFEOF gzeof
#  define mitkIpPicFClose gzclose
#  include "itk_zlib.h"
#else
#  define mitkIpPicFile_t gzFile
#  define mitkIpPicFOpen gzopen
#  define mitkIpPicFTell gztell
#  define mitkIpPicFSeek gzseek
#  define mitkIpPicFRead(buf, bsize, count, file) gzread((file),(buf),((count)*(bsize)))
#  define mitkIpPicFWrite _mitkIpPicFWrite
#  define mitkIpPicFEOF gzeof
#  define mitkIpPicFClose gzclose
#  include <zlib.h>
#endif
#else
#  define mitkIpPicFile_t FILE *
#  define mitkIpPicFOpen fopen
#  define mitkIpPicFTell ftell
#  define mitkIpPicFSeek fseek
#  define mitkIpPicFRead fread
#  define mitkIpPicFWrite fwrite
#  define mitkIpPicFEOF feof
#  define mitkIpPicFClose fclose
#endif


#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpPicFReadLE mitkIpPicFRead
#  define mitkIpPicFReadBE mitkIpPicFReadCvt
#else
#  define mitkIpPicFReadLE mitkIpPicFReadCvt
#  define mitkIpPicFReadBE mitkIpPicFRead
#endif

#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpPicFWriteLE mitkIpPicFWrite
#  define mitkIpPicFWriteBE mitkIpPicFWriteCvt
#else
#  define mitkIpPicFWriteLE mitkIpPicFWriteCvt
#  define mitkIpPicFWriteBE mitkIpPicFWrite
#endif


#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpFReadLE fread
#  define mitkIpFReadBE mitkIpFReadCvt
#else
#  define mitkIpFReadLE mitkIpFReadCvt
#  define mitkIpFReadBE  fread
#endif

#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpFWriteLE fwrite
#  define mitkIpFWriteBE mitkIpFWriteCvt
#else
#  define mitkIpFWriteLE mitkIpFWriteCvt
#  define mitkIpFWriteBE fwrite
#endif


#ifdef WIN
#  undef mitkIpFReadLE
#  define mitkIpFReadLE fread
#endif



#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpCvtFromLE(data, len, bytes)
#  define mitkIpCvtFromBE(data, len, bytes) _mitkIpCvtEndian( (data), (len), (bytes) )
#else
#  define mitkIpCvtFromLE(data, len, bytes) _mitkIpCvtEndian( (data), (len), (bytes) )
#  define mitkIpCvtFromBE(data, len, bytes)
#endif

#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpCpFromLE(source, destination, len, bytes) _mitkIpCp( (source), (destination), (len) )
#  define mitkIpCpFromBE(source, destination, len, bytes) _mitkIpCpCvtEndian( (source), (destination), (len), (bytes) )
#else
#  define mitkIpCpFromLE(source, destination, len, bytes) _mitkIpCpCvtEndian( (source), (destination), (len), (bytes) )
#  define mitkIpCpFromBE(source, destination, len, bytes) _mitkIpCp( (source), (destination), (len) )
#endif

#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpCpToLE(source, destination, len, bytes) _mitkIpCp( (source), (destination), (len) )
#  define mitkIpCpToBE(source, destination, len, bytes) _mitkIpCpCvtEndian( (source), (destination), (len), (bytes) )
#else
#  define mitkIpCpToLE(source, destination, len, bytes) _mitkIpCpCvtEndian( (source), (destination), (len), (bytes) )
#  define mitkIpCpToBE(source, destination, len, bytes) _mitkIpCp( (source), (destination), (len) )
#endif


#define mitkIpEndianSwap2(x) ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

#define mitkIpEndianSwap4(x) ((((x) >> 24) & 0x000000FF) | (((x) << 24) & 0xFF000000)) \
                         | ((((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000))

/*
** NOTE: these macros *DO NOT* work for in-place transformations.
*/
/*
** maybe ipEndianSwap should be used
*/
#ifdef _mitkIpENDIAN_IS_LITTLE
#  define mitkIpInt2FromLE(from, to) *((mitkIpInt2_t *)(to)) = *((mitkIpInt2_t *)(from))
#  define mitkIpInt4FromLE(from, to) *((mitkIpInt4_t *)(to)) = *((mitkIpInt4_t *)(from))
#  define mitkIpUInt2FromLE(from, to) mitkIpInt2FromLE(from, to)
#  define mitkIpUInt4FromLE(from, to) mitkIpInt4FromLE(from, to)
#  define mitkIpFloat4FromLE(from, to) mitkIpInt4FromLE(from, to)
#  define mitkIpFloat8FromLE(from, to) *((mitkIpFloat8_t *)(to)) = *((mitkIpFloat8_t *)(from))
#else
#  define mitkIpInt2FromLE(from, to)  ((char *)(to))[0] = ((char *)(from))[1]; \
                                  ((char *)(to))[1] = ((char *)(from))[0];

#  define mitkIpInt4FromLE(from, to)  ((char *)(to))[0] = ((char *)(from))[3]; \
                                  ((char *)(to))[1] = ((char *)(from))[2]; \
                                  ((char *)(to))[2] = ((char *)(from))[1]; \
                                  ((char *)(to))[3] = ((char *)(from))[0];
#  define mitkIpUInt2FromLE(from, to) mitkIpInt2FromLE(from, to)
#  define mitkIpUInt4FromLE(from, to) mitkIpInt4FromLE(from, to)
#  define mitkIpFloat4FromLE(from, to) mitkIpInt4FromLE(from, to)
#  define mitkIpFloat8FromLE(from, to)  ((char *)(to))[0] = ((char *)(from))[7]; \
                                    ((char *)(to))[1] = ((char *)(from))[6]; \
                                    ((char *)(to))[2] = ((char *)(from))[5]; \
                                    ((char *)(to))[3] = ((char *)(from))[4]; \
                                    ((char *)(to))[4] = ((char *)(from))[3]; \
                                    ((char *)(to))[5] = ((char *)(from))[2]; \
                                    ((char *)(to))[6] = ((char *)(from))[1]; \
                                    ((char *)(to))[7] = ((char *)(from))[0];
#endif /* _mitkIpENDIAN_IS_LITTLE */

/*
** NOTE: Most architectures are symmetrical with respect to conversions.
*/
#define mitkIpInt2ToLE(from, to) mitkIpInt2FromLE(from, to)
#define mitkIpInt4ToLE(from, to) mitkIpInt4FromLE(from, to)
#define mitkIpUInt2ToLE(from, to) mitkIpUInt2FromLE(from, to)
#define mitkIpUInt4ToLE(from, to) mitkIpUInt4FromLE(from, to)
#define mitkIpFloat4ToLE(from, to) mitkIpFloat4FromLE(from, to)
#define mitkIpFloat8ToLE(from, to) mitkIpFloat8FromLE(from, to)


/*
** types for the pic format
*/

typedef char  mitkIpPicTag_t[_mitkIpPicTAGLEN];

typedef enum
  {
    mitkIpPicUnknown       =   0,
    mitkIpPicBool          =   1,
    mitkIpPicASCII         =   2,
    mitkIpPicInt           =   3,
    mitkIpPicUInt          =   4,
    mitkIpPicFloat         =   5,
    mitkIpPicNonUniform    =   7,
    mitkIpPicTSV           =   8,
    /* ----          add new types before this line !!!            ----*/
    /* ---- don't forget to add a new type also to mitkIpPicType.c !!! ----*/
    _mitkIpPicTypeMax
  } mitkIpPicType_t;

/*
typedef enum
  {
    mitkIpPicUnknown_m     =   0,
    mitkIpPicDescriptor_m  =   0xDDDD
  } _mitkIpPicMagic_t;
*/


typedef struct
  {
             char  tag[_mitkIpPicTAGLEN+1]; /* name of this tag                 */
      mitkIpPicType_t  type;                /* datatype of the value            */
        mitkIpUInt4_t  bpe;                 /* bits per element                 */
        mitkIpUInt4_t  dim;                 /* number of dimensions             */
        mitkIpUInt4_t  n[_mitkIpPicNDIM];       /* size of dimension n[i]           */
             void *value;               /* the value                        */
  } mitkIpPicTSV_t;                         /* Tag Structure Value              */

typedef struct _mitkIpPicTagsElement_s
  {
                    mitkIpPicTSV_t *tsv;
    struct _mitkIpPicTagsElement_s *prev;
    struct _mitkIpPicTagsElement_s *next;
  } _mitkIpPicTagsElement_t;

typedef struct
  {
             mitkIpPicTag_t  version;
    _mitkIpPicTagsElement_t *tags_head;
               mitkIpBool_t  write_protect;
                   long  pixel_start_in_file;
 struct mitkIpPicDescriptor *lender;              /* lender of image data    */
              mitkIpUInt4_t  borrower;            /* borrower of image data  */ 
              mitkIpUInt4_t  ref;                 /* reference counter       */
  } _mitkIpPicInfo_t;

typedef struct mitkIpPicDescriptor
  {
             void *data;          /* pointer to 'image' data          */
     _mitkIpPicInfo_t *info;          /* pointer to the PicInfo structure */
      mitkIpPicType_t  type;          /* datatype of the data             */
        mitkIpUInt4_t  bpe;           /* bits per element                 */
        mitkIpUInt4_t  dim;           /* number of dimensions             */
        mitkIpUInt4_t  n[_mitkIpPicNDIM]; /* size of dimension n[i]           */
#ifdef WIN
           HANDLE  hdata;
#endif
  } mitkIpPicDescriptor;


#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern _mitkIpEndian_t _mitkIpEndian( void );

extern void _mitkIpCp( void *source, void *destination, unsigned long int len );
extern void _mitkIpCvtEndian( void *data, unsigned long int len, unsigned char bytes );
extern void _mitkIpCpCvtEndian( void *source, void *destination, unsigned long int len, unsigned char bytes );

extern mitkIpPicFile_t _mitkIpPicOpenPicFileIn( const char *path );
extern int mitkIpPicAccess( const char *path, int mode );
extern int mitkIpPicRemoveFile( const char *path );

extern mitkIpBool_t mitkIpPicGetWriteCompression( void );
extern mitkIpBool_t mitkIpPicSetWriteCompression( mitkIpBool_t compression );

extern size_t mitkIpFReadCvt( void *ptr, size_t size, size_t nitems, FILE *stream );
extern size_t _mitkIpFWrite( void *ptr, size_t size, size_t nitems, FILE *stream );

#define _mitkIpPicHAS_CAN_READ_COMPRESSED
extern mitkIpBool_t _mitkIpPicCanReadCompressed;
extern size_t mitkIpPicFReadCvt( void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream );

#define _mitkIpPicHAS_CAN_WRITE_COMPRESSED
extern mitkIpBool_t _mitkIpPicCanWriteCompressed;
extern size_t mitkIpPicFWriteCvt( void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream );

extern mitkIpPicDescriptor *mitkIpPicNew( void );
extern void mitkIpPicClear( mitkIpPicDescriptor *pic );
extern void mitkIpPicFree( mitkIpPicDescriptor *pic );

extern mitkIpUInt4_t _mitkIpPicSize(
#if defined(__cplusplus) || defined(c_plusplus)
                             const
#endif
                             mitkIpPicDescriptor *pic );

extern mitkIpUInt4_t _mitkIpPicElements( mitkIpPicDescriptor *pic );

extern mitkIpUInt4_t _mitkIpPicTSVSize( mitkIpPicTSV_t *tsv );
extern mitkIpUInt4_t _mitkIpPicTSVElements( mitkIpPicTSV_t *tsv );

extern mitkIpPicDescriptor *mitkIpPicClone( mitkIpPicDescriptor *pic );

extern mitkIpPicDescriptor *_mitkIpPicCopySlice( mitkIpPicDescriptor *pic, mitkIpPicDescriptor *pic_in, mitkIpUInt4_t slice );

extern mitkIpPicDescriptor *_mitkIpPicOldGet( FILE *infile, mitkIpPicDescriptor *pic );
extern mitkIpPicDescriptor *_mitkIpPicOldGetHeader( FILE *infile, mitkIpPicDescriptor *pic );
extern mitkIpPicDescriptor *_mitkIpPicOldGetSlice( FILE *infile, mitkIpPicDescriptor *pic, mitkIpUInt4_t slice );


extern mitkIpPicDescriptor *mitkIpPicCopyHeader( mitkIpPicDescriptor *pic, mitkIpPicDescriptor *pic_new );

extern mitkIpPicDescriptor *mitkIpPicGet( const char *picfile_name, mitkIpPicDescriptor *pic );
extern mitkIpPicDescriptor *mitkIpPicGetHeader( const char *picfile_name, mitkIpPicDescriptor *pic );
extern mitkIpPicDescriptor *mitkIpPicGetSlice( const char *picfile_name, mitkIpPicDescriptor *pic, mitkIpUInt4_t slice );

extern _mitkIpPicTagsElement_t *_mitkIpPicReadTags( _mitkIpPicTagsElement_t *head, mitkIpUInt4_t bytes_to_read, FILE *stream, char encryption_type );
extern _mitkIpPicTagsElement_t *_mitkIpPicInsertTag( _mitkIpPicTagsElement_t *head, mitkIpPicTSV_t *tsv );
extern _mitkIpPicTagsElement_t *_mitkIpPicFindTag( _mitkIpPicTagsElement_t *head, const char *tag );
extern mitkIpUInt4_t _mitkIpPicTagsSize( _mitkIpPicTagsElement_t *head );
extern mitkIpUInt4_t _mitkIpPicTagsNumber( _mitkIpPicTagsElement_t *head );
extern void _mitkIpPicWriteTags( _mitkIpPicTagsElement_t *head, FILE *stream, char encryption_type );
extern void _mitkIpPicFreeTags( _mitkIpPicTagsElement_t *head );
extern mitkIpPicTSV_t *_mitkIpPicCloneTag( mitkIpPicTSV_t *source );
extern _mitkIpPicTagsElement_t *_mitkIpPicCloneTags( _mitkIpPicTagsElement_t *head );

extern void mitkIpPicAddTag( mitkIpPicDescriptor *pic, mitkIpPicTSV_t *tsv );
extern void mitkIpPicAddSubTag( mitkIpPicTSV_t *parent, mitkIpPicTSV_t *tsv );
extern mitkIpPicTSV_t * mitkIpPicDelTag( mitkIpPicDescriptor *pic, const char *tag );
extern mitkIpPicTSV_t * mitkIpPicDelSubTag( mitkIpPicTSV_t *parent, const char *tag );
extern mitkIpPicDescriptor *mitkIpPicGetTags( const char *picfile_name, mitkIpPicDescriptor *pic );
extern mitkIpPicTSV_t *mitkIpPicQueryTag( mitkIpPicDescriptor *pic, const char *t );
extern mitkIpPicTSV_t *mitkIpPicQuerySubTag( mitkIpPicTSV_t *parent, const char *t );
extern void mitkIpPicFreeTag( mitkIpPicTSV_t *tsv );


extern _mitkIpPicTagsElement_t * _mitkIpPicReadTagsMem( _mitkIpPicTagsElement_t *head, mitkIpUInt4_t bytes_to_read, mitkIpUInt1_t **mem_ptr, char encryption_type );
extern mitkIpPicDescriptor *mitkIpPicGetMem( mitkIpUInt1_t *mem_pic );
extern void _mitkIpPicWriteTagsMem( _mitkIpPicTagsElement_t *head, mitkIpUInt1_t **mem_ptr );
extern mitkIpUInt1_t *mitkIpPicPutMem( mitkIpPicDescriptor *pic, int *mem_size );


extern int mitkIpPicPut( const char *picfile_name, mitkIpPicDescriptor *pic );
extern void mitkIpPicPutSlice( const char *picfile_name, mitkIpPicDescriptor *pic, mitkIpUInt4_t slice );

#define _mitkIpPicInfoNORMAL	0
#define _mitkIpPicInfoSHORT		(1 << 0)
void _mitkIpPicInfo( FILE *stream, mitkIpPicDescriptor *pic, mitkIpUInt4_t flags );
void _mitkIpPicInfoPrintTags( FILE *stream, _mitkIpPicTagsElement_t* head, mitkIpUInt4_t level, mitkIpUInt4_t flags );

extern const char *mitkIpPicTypeName( mitkIpUInt4_t type );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif  /* _mitkIpPic_h */

#ifdef MITK_UNMANGLE_IPPIC
#include "mitkIpPicUnmangle.h"
#endif
