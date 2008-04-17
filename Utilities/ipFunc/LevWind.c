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
 *  this function performs a level-window operation
 */

/** @brief this function performs a level-window operation
 *
 *  All greyvalues which are not included in the 
 *  intervall [level-0.5*window, level+0.5*window] are set to 
 *  the minimal (if greyvalue<level-0.5*window) or maximal 
 *  greyvalue (if greyvalue>level-0.5*window), respectively.
 *
 *  @param pic_old  pointer to original image
 *  @param level    greyvalue that forms center of the window
 *  @param window   length of window
 *  @param pic_return memory which could be used for pic_new. If
 *               pic_retrun == NULL new memory is allocated
 *
 *  @return pointer to the transformed image
 *
 *  @par Uses
 *  @arg ipFuncSelMM
 *
 * AUTHOR & DATE
 */

/* include files                                                              */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncLevWin ( ipPicDescriptor *pic_old,
                                ipFloat8_t      level, 
                                ipFloat8_t      window,
                                ipPicDescriptor *pic_return );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncLevWin\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* -------------------------------------------------------------------------  */
/*
** ipFunclevWin
*/
/* -------------------------------------------------------------------------  */

ipPicDescriptor *ipFuncLevWin ( ipPicDescriptor *pic_old,
                                ipFloat8_t      level, 
                                ipFloat8_t      window,
                                ipPicDescriptor *pic_return )
{
   ipPicDescriptor *pic_new;         /* pointer to new image                  */ 
   ipFloat8_t      gv_low;           /* lower greyvalue of range              */
   ipFloat8_t      gv_up;            /* upper greyvalue of range              */
   ipFloat8_t      max_gv, min_gv;   /* max and min possible greyvalues       */

   /* check image data                                                        */

   if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

   /* calculate max and min possible greyvalues                               */

   if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
     return ( mitkIpFuncERROR );
   /* calculate lower and upper greyvalue of range with level and window      */

   gv_low = level - 0.5 * window;

   if ( gv_low < min_gv )
     gv_low = min_gv;
   else if ( gv_low > max_gv )
     gv_low = max_gv;

   gv_up  = level + 0.5 * window; 

   if ( gv_up < min_gv )
     gv_up = min_gv;
   else if ( gv_up > max_gv )
     gv_up = max_gv;

   /* calculate new image in Function ipFuncSelect                            */

   pic_new = ipFuncSelMM ( pic_old, gv_low, gv_up, pic_return  );

   /* Copy Tags */
 
   ipFuncCopyTags(pic_new, pic_old);
   
   
                              

   return ( pic_new );
}
#endif

