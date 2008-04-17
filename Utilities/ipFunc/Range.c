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
 *  this function checks whether the greyvalues of the image are in the 
 *  spezified range
 */

/** @brief checks whether the greyvalues of the image are in the 
 *  spezified range
 *
 *  @param pic_old   pointer to original image
 *  @param gv_low    lower end of range
 *  @param gv_up     upper end of range
 *
 *  @return mitkIpFuncOK   when greyvalues are in specified range
 *  @return mitkIpFuncERROR  when greyvalues are not in specigfied range or an error 
 *              occured
 *
 * USES
 *  function ipFuncExtr : to calculate extreme greyvalues of the image
 *
 * AUTHOR & DATE
 */

/* include files                                                            */

#include "ipFuncP.h"

ipUInt4_t  ipFuncRange ( ipPicDescriptor *pic_old,
                         ipFloat8_t      gv_low,
                         ipFloat8_t      gv_up );
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/* ------------------------------------------------------------------------ */
/*
** function ipFuncRange
*/
/* ------------------------------------------------------------------------ */

ipUInt4_t  ipFuncRange ( ipPicDescriptor *pic_old,
                         ipFloat8_t      gv_low,
                         ipFloat8_t      gv_up )
{
  ipFloat8_t       min, max;        /* extreme greyvalues                   */

  /* check image data                                                       */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( gv_low > gv_up )
    { 
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* calculate max. and min. possible greyvalues                            */

  if ( ipFuncExtr ( pic_old, &min, &max) == mitkIpFuncERROR ) return ( mitkIpFuncERROR );

  if ( min > gv_low || max < gv_up ) return ( mitkIpFuncOK );
  else return ( mitkIpFuncERROR );

}
#endif

