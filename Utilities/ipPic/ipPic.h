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
 * moved win32 compatibility settings from ipPic.h to ipPic.pro
 *
 * Revision 1.39  2004/03/25 18:35:05  andre
 * bug fix
 *
 * Revision 1.38  2003/12/02 11:54:31  tobiask
 * Added setters and getters for rgb pixels.
 *
 * Revision 1.37  2003/11/28 14:24:45  tobiask
 * Fixed bugs in macros ipPicGET_PIXEL and ipPicPUT_PIXEL.
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
 * changed include path to ipPic/ipPic.h
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
 * added ipPicDelSubTag
 *
 * Revision 1.7  1998/06/09  11:34:32  andre
 * *** empty log message ***
 *
 * Revision 1.6  1998/06/09  10:58:25  andre
 * added ipPicAddSubTag
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
 * ipPicPut now returns status
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

#include <mitkConfig.h>

#if defined ( MITK_CHILI_PLUGIN ) && ( ! defined ( MITK_IPPIC_COMPILE ) )

#include MITK_CHILI_PLUGIN_SDK_IPTYPES_H
#include MITK_CHILI_PLUGIN_SDK_IPPIC_H

#else

#ifndef _ipPic_h
#define _ipPic_h

#ifndef NO_DEBUG
#  define ipPicDEBUG
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

#include <ipPic/ipTypes.h>


#ifdef ipPicDEBUG
#endif


/*
** defines
*/
#define ipPicVERSION "PIC VERSION 3.00                "
#define ipPicVERSIONe "PIC VERSION 3.00e               "
#define _ipPicNDIM 8
#define _ipPicTAGLEN 32


#if (THIS_IS_BIG_ENDIAN) || defined (_IBMR2) || (mc68000) || (sun) || (__convex__) || (__hppa) || (_MIPSEB) || defined ( __PPC__) || defined (__APPLE__)
#  define _ipENDIAN_IS_BIG
#else
#  define _ipENDIAN_IS_LITTLE
#endif


#define ipPicVersionMajor(pic) (ipUInt1_t)(pic->info->version[12] - '0')
#define ipPicVersionMinor(pic) (ipUInt1_t)(10 * (pic->info->version[14]-'0' ) \
                                           + pic->info->version[15]-'0')
#define ipPicEncryptionType(pic) (pic->info->version[16])

/*#define ipPicIsPic(pic) (( (pic)->magic == ipPicPicDescriptor_m) \
                             ? ipTrue : ipFalse )*/


#define ipPicDR(type,len) (ipUInt4_t)( ((type)<<16) + (len) )

#define ipPicFOR_TYPE( _ipPIC_TYPE, _ipPIC_CMD, _ipPIC_PIC) \
  _ipPIC_CMD(_ipPIC_TYPE,_ipPIC_PIC)

#define ipPicFORALL( _ipPIC_CMD, _ipPIC_PIC )                                    \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC);                                        \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC);                                       \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC);                                        \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC);                                       \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC);                                        \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC);                                       \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC);                                      \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC);                                      \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define ipPicFORALL_1( _ipPIC_CMD, _ipPIC_PIC, A1 )                              \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC, A1);                                    \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC, A1);                                   \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC, A1);                                    \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC, A1);                                   \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC, A1);                                    \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC, A1);                                   \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC, A1);                                  \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC, A1);                                  \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define ipPicFORALL_2( _ipPIC_CMD, _ipPIC_PIC, A1, A2 )                          \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC, A1, A2);                                \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC, A1, A2);                               \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC, A1, A2);                                \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC, A1, A2);                               \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC, A1, A2);                                \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC, A1, A2);                               \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC, A1, A2);                              \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC, A1, A2);                              \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define ipPicFORALL_3( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3 )                      \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC, A1, A2, A3);                            \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC, A1, A2, A3);                           \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC, A1, A2, A3);                            \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC, A1, A2, A3);                           \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC, A1, A2, A3);                            \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC, A1, A2, A3);                           \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC, A1, A2, A3);                          \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC, A1, A2, A3);                          \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define ipPicFORALL_4( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3, A4 )                  \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC, A1, A2, A3, A4);                        \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC, A1, A2, A3, A4);                       \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC, A1, A2, A3, A4);                        \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC, A1, A2, A3, A4);                       \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC, A1, A2, A3, A4);                        \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC, A1, A2, A3, A4);                       \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC, A1, A2, A3, A4);                      \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC, A1, A2, A3, A4);                      \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define ipPicFORALL_5( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3, A4, A5 )              \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                    \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                   \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                    \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                   \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                    \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                   \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                  \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC, A1, A2, A3, A4, A5);                  \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define ipPicFORALL_6( _ipPIC_CMD, _ipPIC_PIC, A1, A2, A3, A4, A5, A6 )          \
  switch( ipPicDR((_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe) )                       \
    {                                                                            \
      case ipPicDR( ipPicInt, 8 ):                                               \
        _ipPIC_CMD(ipInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);                \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 8 ):                                              \
        _ipPIC_CMD(ipUInt1_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);               \
        break;                                                                   \
      case ipPicDR( ipPicInt, 16 ):                                              \
        _ipPIC_CMD(ipInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);                \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 16 ):                                             \
        _ipPIC_CMD(ipUInt2_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);               \
        break;                                                                   \
      case ipPicDR( ipPicInt, 32 ):                                              \
        _ipPIC_CMD(ipInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);                \
        break;                                                                   \
      case ipPicDR( ipPicUInt, 32 ):                                             \
        _ipPIC_CMD(ipUInt4_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);               \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 32 ):                                            \
        _ipPIC_CMD(ipFloat4_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);              \
        break;                                                                   \
      case ipPicDR( ipPicFloat, 64 ):                                            \
        _ipPIC_CMD(ipFloat8_t, _ipPIC_PIC, A1, A2, A3, A4, A5, A6);              \
        break;                                                                   \
      default:                                                                   \
        fprintf( stderr,                                                         \
                 "sorry, %s: datatype (%i/%i) not supported for %s.\n",          \
                 __FILE__, (_ipPIC_PIC)->type, (_ipPIC_PIC)->bpe, #_ipPIC_CMD ); \
          break;                                                                 \
    }

#define _ipPicGET_PIXEL(TYPE,PIC,X,Y,RESULT) \
{                                            \
  TYPE* data = (TYPE *) (PIC)->data          \
  + (X) + (Y) * (PIC)->n[0];                 \
  RESULT = *data;                            \
}

#define _ipPicPUT_PIXEL(TYPE,PIC,X,Y,VALUE)  \
{                                            \
  TYPE* data = (TYPE *) (PIC)->data          \
  + (X) + (Y) * (PIC)->n[0];                 \
  *data = VALUE;                             \
}

#define ipPicGET_PIXEL(PIC,X,Y,RESULT)             \
{                                                  \
  ipPicFORALL_3(_ipPicGET_PIXEL,PIC,X,Y,RESULT);   \
}

#define ipPicPUT_PIXEL(PIC,X,Y,RESULT)             \
{                                                  \
  ipPicFORALL_3(_ipPicPUT_PIXEL,PIC,X,Y,RESULT);   \
}

#define _ipPicGET_PIXEL_RGB(TYPE,PIC,X,Y,R,G,B)                \
{                                                              \
  ipUInt4_t skip = (PIC)->n[0] * (PIC)->n[1];                  \
  TYPE* data = (TYPE *) (PIC)->data + (X) + (Y) * (PIC)->n[0]; \
  R = *(data);                                                 \
  G = *(data + skip);                                          \
  B = *(data + skip + skip);                                   \
}

#define _ipPicPUT_PIXEL_RGB(TYPE,PIC,X,Y,R,G,B)                \
{                                                              \
  ipUInt4_t skip = (PIC)->n[0] * (PIC)->n[1];                  \
  TYPE* data = (TYPE *) (PIC)->data + (X) + (Y) * (PIC)->n[0]; \
  *data = (R);                                                 \
  *(data + skip) = (G);                                        \
  *(data + skip + skip) = (B);                                 \
}

#define ipPicGET_PIXEL_RGB(PIC,X,Y,R,G,B)             \
{                                                     \
  ipPicFORALL_5(_ipPicGET_PIXEL_RGB,PIC,X,Y,R,G,B);   \
}

#define ipPicPUT_PIXEL_RGB(PIC,X,Y,R,G,B)             \
{                                                     \
  ipPicFORALL_5(_ipPicPUT_PIXEL_RGB,PIC,X,Y,R,G,B);   \
}

#define USE_ZLIB /**/
#ifdef USE_ZLIB
#ifdef USE_ITKZLIB
#  define ipPicFile_t gzFile
#  define ipPicFOpen gzopen
#  define ipPicFTell gztell
#  define ipPicFSeek gzseek
#  define ipPicFRead(buf, bsize, count, file) gzread((file),(buf),((count)*(bsize)))
#  define ipPicFWrite _ipPicFWrite
#  define ipPicFEOF gzeof
#  define ipPicFClose gzclose
#  include "itk_zlib.h"
#else
#  define ipPicFile_t gzFile
#  define ipPicFOpen gzopen
#  define ipPicFTell gztell
#  define ipPicFSeek gzseek
#  define ipPicFRead(buf, bsize, count, file) gzread((file),(buf),((count)*(bsize)))
#  define ipPicFWrite _ipPicFWrite
#  define ipPicFEOF gzeof
#  define ipPicFClose gzclose
#  include <zlib.h>
#endif
#else
#  define ipPicFile_t FILE *
#  define ipPicFOpen fopen
#  define ipPicFTell ftell
#  define ipPicFSeek fseek
#  define ipPicFRead fread
#  define ipPicFWrite fwrite
#  define ipPicFEOF feof
#  define ipPicFClose fclose
#endif


#ifdef _ipENDIAN_IS_LITTLE
#  define ipPicFReadLE ipPicFRead
#  define ipPicFReadBE ipPicFReadCvt
#else
#  define ipPicFReadLE ipPicFReadCvt
#  define ipPicFReadBE ipPicFRead
#endif

#ifdef _ipENDIAN_IS_LITTLE
#  define ipPicFWriteLE ipPicFWrite
#  define ipPicFWriteBE ipPicFWriteCvt
#else
#  define ipPicFWriteLE ipPicFWriteCvt
#  define ipPicFWriteBE ipPicFWrite
#endif


#ifdef _ipENDIAN_IS_LITTLE
#  define ipFReadLE fread
#  define ipFReadBE ipFReadCvt
#else
#  define ipFReadLE ipFReadCvt
#  define ipFReadBE fread
#endif

#ifdef _ipENDIAN_IS_LITTLE
#  define ipFWriteLE fwrite
#  define ipFWriteBE ipFWriteCvt
#else
#  define ipFWriteLE ipFWriteCvt
#  define ipFWriteBE fwrite
#endif


#ifdef WIN
#  undef ipFReadLE
#  define ipFReadLE fread
#endif



#ifdef _ipENDIAN_IS_LITTLE
#  define ipCvtFromLE(data, len, bytes)
#  define ipCvtFromBE(data, len, bytes) _ipCvtEndian( (data), (len), (bytes) )
#else
#  define ipCvtFromLE(data, len, bytes) _ipCvtEndian( (data), (len), (bytes) )
#  define ipCvtFromBE(data, len, bytes)
#endif

#ifdef _ipENDIAN_IS_LITTLE
#  define ipCpFromLE(source, destination, len, bytes) _ipCp( (source), (destination), (len) )
#  define ipCpFromBE(source, destination, len, bytes) _ipCpCvtEndian( (source), (destination), (len), (bytes) )
#else
#  define ipCpFromLE(source, destination, len, bytes) _ipCpCvtEndian( (source), (destination), (len), (bytes) )
#  define ipCpFromBE(source, destination, len, bytes) _ipCp( (source), (destination), (len) )
#endif

#ifdef _ipENDIAN_IS_LITTLE
#  define ipCpToLE(source, destination, len, bytes) _ipCp( (source), (destination), (len) )
#  define ipCpToBE(source, destination, len, bytes) _ipCpCvtEndian( (source), (destination), (len), (bytes) )
#else
#  define ipCpToLE(source, destination, len, bytes) _ipCpCvtEndian( (source), (destination), (len), (bytes) )
#  define ipCpToBE(source, destination, len, bytes) _ipCp( (source), (destination), (len) )
#endif


#define ipEndianSwap2(x) ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

#define ipEndianSwap4(x) ((((x) >> 24) & 0x000000FF) | (((x) << 24) & 0xFF000000)) \
                         | ((((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000))

/*
** NOTE: these macros *DO NOT* work for in-place transformations.
*/
/*
** maybe ipEndianSwap should be used
*/
#ifdef _ipENDIAN_IS_LITTLE
#  define ipInt2FromLE(from, to) *((ipInt2_t *)(to)) = *((ipInt2_t *)(from))
#  define ipInt4FromLE(from, to) *((ipInt4_t *)(to)) = *((ipInt4_t *)(from))
#  define ipUInt2FromLE(from, to) ipInt2FromLE(from, to)
#  define ipUInt4FromLE(from, to) ipInt4FromLE(from, to)
#  define ipFloat4FromLE(from, to) ipInt4FromLE(from, to)
#  define ipFloat8FromLE(from, to) *((ipFloat8_t *)(to)) = *((ipFloat8_t *)(from))
#else
#  define ipInt2FromLE(from, to)  ((char *)(to))[0] = ((char *)(from))[1]; \
                                  ((char *)(to))[1] = ((char *)(from))[0];

#  define ipInt4FromLE(from, to)  ((char *)(to))[0] = ((char *)(from))[3]; \
                                  ((char *)(to))[1] = ((char *)(from))[2]; \
                                  ((char *)(to))[2] = ((char *)(from))[1]; \
                                  ((char *)(to))[3] = ((char *)(from))[0];
#  define ipUInt2FromLE(from, to) ipInt2FromLE(from, to)
#  define ipUInt4FromLE(from, to) ipInt4FromLE(from, to)
#  define ipFloat4FromLE(from, to) ipInt4FromLE(from, to)
#  define ipFloat8FromLE(from, to)  ((char *)(to))[0] = ((char *)(from))[7]; \
                                    ((char *)(to))[1] = ((char *)(from))[6]; \
                                    ((char *)(to))[2] = ((char *)(from))[5]; \
                                    ((char *)(to))[3] = ((char *)(from))[4]; \
                                    ((char *)(to))[4] = ((char *)(from))[3]; \
                                    ((char *)(to))[5] = ((char *)(from))[2]; \
                                    ((char *)(to))[6] = ((char *)(from))[1]; \
                                    ((char *)(to))[7] = ((char *)(from))[0];
#endif /* _ipENDIAN_IS_LITTLE */

/*
** NOTE: Most architectures are symmetrical with respect to conversions.
*/
#define ipInt2ToLE(from, to) ipInt2FromLE(from, to)
#define ipInt4ToLE(from, to) ipInt4FromLE(from, to)
#define ipUInt2ToLE(from, to) ipUInt2FromLE(from, to)
#define ipUInt4ToLE(from, to) ipUInt4FromLE(from, to)
#define ipFloat4ToLE(from, to) ipFloat4FromLE(from, to)
#define ipFloat8ToLE(from, to) ipFloat8FromLE(from, to)


/*
** types for the pic format
*/

typedef char  ipPicTag_t[_ipPicTAGLEN];

typedef enum
  {
    ipPicUnknown       =   0,
    ipPicBool          =   1,
    ipPicASCII         =   2,
    ipPicInt           =   3,
    ipPicUInt          =   4,
    ipPicFloat         =   5,
    ipPicNonUniform    =   7,
    ipPicTSV           =   8,
    /* ----          add new types before this line !!!            ----*/
    /* ---- don't forget to add a new type also to ipPicType.c !!! ----*/
    _ipPicTypeMax
  } ipPicType_t;

/*
typedef enum
  {
    ipPicUnknown_m     =   0,
    ipPicDescriptor_m  =   0xDDDD
  } _ipPicMagic_t;
*/


typedef struct
  {
             char  tag[_ipPicTAGLEN+1]; /* name of this tag                 */
      ipPicType_t  type;                /* datatype of the value            */
        ipUInt4_t  bpe;                 /* bits per element                 */
        ipUInt4_t  dim;                 /* number of dimensions             */
        ipUInt4_t  n[_ipPicNDIM];       /* size of dimension n[i]           */
             void *value;               /* the value                        */
  } ipPicTSV_t;                         /* Tag Structure Value              */

typedef struct _ipPicTagsElement_s
  {
                    ipPicTSV_t *tsv;
    struct _ipPicTagsElement_s *prev;
    struct _ipPicTagsElement_s *next;
  } _ipPicTagsElement_t;

typedef struct
  {
             ipPicTag_t  version;
    _ipPicTagsElement_t *tags_head;
               ipBool_t  write_protect;
                   long  pixel_start_in_file;
  } _ipPicInfo_t;

typedef struct ipPicDescriptor
  {
             void *data;          /* pointer to 'image' data          */
     _ipPicInfo_t *info;          /* pointer to the PicInfo structure */
      ipPicType_t  type;          /* datatype of the data             */
        ipUInt4_t  bpe;           /* bits per element                 */
        ipUInt4_t  dim;           /* number of dimensions             */
        ipUInt4_t  n[_ipPicNDIM]; /* size of dimension n[i]           */
#ifdef WIN
           HANDLE  hdata;
#endif
  } ipPicDescriptor;


#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern _ipEndian_t _ipEndian( void );

extern void _ipCp( void *source, void *destination, unsigned long int len );
extern void _ipCvtEndian( void *data, unsigned long int len, unsigned char bytes );
extern void _ipCpCvtEndian( void *source, void *destination, unsigned long int len, unsigned char bytes );

extern ipPicFile_t _ipPicOpenPicFileIn( const char *path );
extern int ipPicAccess( const char *path, int mode );
extern int ipPicRemoveFile( const char *path );

extern ipBool_t ipPicGetWriteCompression( void );
extern ipBool_t ipPicSetWriteCompression( ipBool_t compression );

extern size_t ipFReadCvt( void *ptr, size_t size, size_t nitems, FILE *stream );
extern size_t _ipFWrite( void *ptr, size_t size, size_t nitems, FILE *stream );

#define _ipPicHAS_CAN_READ_COMPRESSED
extern ipBool_t _ipPicCanReadCompressed;
extern size_t ipPicFReadCvt( void *ptr, size_t size, size_t nitems, ipPicFile_t stream );

#define _ipPicHAS_CAN_WRITE_COMPRESSED
extern ipBool_t _ipPicCanWriteCompressed;
extern size_t ipPicFWriteCvt( void *ptr, size_t size, size_t nitems, ipPicFile_t stream );

extern ipPicDescriptor *ipPicNew( void );
extern void ipPicClear( ipPicDescriptor *pic );
extern void ipPicFree( ipPicDescriptor *pic );

extern ipUInt4_t _ipPicSize(
#if defined(__cplusplus) || defined(c_plusplus)
                             const
#endif
                             ipPicDescriptor *pic );

extern ipUInt4_t _ipPicElements( ipPicDescriptor *pic );

extern ipUInt4_t _ipPicTSVSize( ipPicTSV_t *tsv );
extern ipUInt4_t _ipPicTSVElements( ipPicTSV_t *tsv );

extern ipPicDescriptor *ipPicClone( ipPicDescriptor *pic );

extern ipPicDescriptor *_ipPicCopySlice( ipPicDescriptor *pic, ipPicDescriptor *pic_in, ipUInt4_t slice );

extern ipPicDescriptor *_ipPicOldGet( FILE *infile, ipPicDescriptor *pic );
extern ipPicDescriptor *_ipPicOldGetHeader( FILE *infile, ipPicDescriptor *pic );
extern ipPicDescriptor *_ipPicOldGetSlice( FILE *infile, ipPicDescriptor *pic, ipUInt4_t slice );


extern ipPicDescriptor *ipPicCopyHeader( ipPicDescriptor *pic, ipPicDescriptor *pic_new );

extern ipPicDescriptor *ipPicGet( const char *picfile_name, ipPicDescriptor *pic );
extern ipPicDescriptor *ipPicGetHeader( const char *picfile_name, ipPicDescriptor *pic );
extern ipPicDescriptor *ipPicGetSlice( const char *picfile_name, ipPicDescriptor *pic, ipUInt4_t slice );

extern _ipPicTagsElement_t *_ipPicReadTags( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, FILE *stream, char encryption_type );
extern _ipPicTagsElement_t *_ipPicInsertTag( _ipPicTagsElement_t *head, ipPicTSV_t *tsv );
extern _ipPicTagsElement_t *_ipPicFindTag( _ipPicTagsElement_t *head, const char *tag );
extern ipUInt4_t _ipPicTagsSize( _ipPicTagsElement_t *head );
extern ipUInt4_t _ipPicTagsNumber( _ipPicTagsElement_t *head );
extern void _ipPicWriteTags( _ipPicTagsElement_t *head, FILE *stream, char encryption_type );
extern void _ipPicFreeTags( _ipPicTagsElement_t *head );
extern ipPicTSV_t *_ipPicCloneTag( ipPicTSV_t *source );
extern _ipPicTagsElement_t *_ipPicCloneTags( _ipPicTagsElement_t *head );

extern void ipPicAddTag( ipPicDescriptor *pic, ipPicTSV_t *tsv );
extern void ipPicAddSubTag( ipPicTSV_t *parent, ipPicTSV_t *tsv );
extern ipPicTSV_t * ipPicDelTag( ipPicDescriptor *pic, const char *tag );
extern ipPicTSV_t * ipPicDelSubTag( ipPicTSV_t *parent, const char *tag );
extern ipPicDescriptor *ipPicGetTags( const char *picfile_name, ipPicDescriptor *pic );
extern ipPicTSV_t *ipPicQueryTag( ipPicDescriptor *pic, const char *t );
extern ipPicTSV_t *ipPicQuerySubTag( ipPicTSV_t *parent, const char *t );
extern void ipPicFreeTag( ipPicTSV_t *tsv );


extern _ipPicTagsElement_t * _ipPicReadTagsMem( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, ipUInt1_t **mem_ptr, char encryption_type );
extern ipPicDescriptor *ipPicGetMem( ipUInt1_t *mem_pic );
extern void _ipPicWriteTagsMem( _ipPicTagsElement_t *head, ipUInt1_t **mem_ptr );
extern ipUInt1_t *ipPicPutMem( ipPicDescriptor *pic, int *mem_size );


extern int ipPicPut( const char *picfile_name, ipPicDescriptor *pic );
extern void ipPicPutSlice( const char *picfile_name, ipPicDescriptor *pic, ipUInt4_t slice );

#define _ipPicInfoNORMAL	0
#define _ipPicInfoSHORT		(1 << 0)
void _ipPicInfo( FILE *stream, ipPicDescriptor *pic, ipUInt4_t flags );
void _ipPicInfoPrintTags( FILE *stream, _ipPicTagsElement_t* head, ipUInt4_t level, ipUInt4_t flags );

extern const char *ipPicTypeName( ipUInt4_t type );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif  /* MITK_CHILI_PLUGIN */
#endif  /* _ipPic_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
