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
 *  this function calculates the standard deviation of all pixels 
 *  inside of a polygon
 */

/** @brief calculates the standard deviation of all pixels 
 *  inside of a polygon
 *
 *  The polygon is described by a sequence of points
 *
 *  @param pic_old   pointer to original image
 *  @param pol_x     vector with the x-coordinates of the points which form
 *              form the roi
 *  @param pol_y     vector with the y-coordinates of the points which form
 *              form the roi
 *  @param no_pts    number of points used to describe ROI
 *
 * REMARKS
 *  this function could just be used for 2D images
 *
 * @return standard deviation of all greyvalues in ROI
 *
 * AUTHOR & DATE
 */

/* include files                                                       */

#include "mitkIpFuncP.h"

ipFloat8_t mitkIpFuncSDevROI ( mitkIpPicDescriptor *pic_old,
                           ipUInt4_t       *pol_x,
                           ipUInt4_t       *pol_y,
                           ipUInt4_t       no_pts ) ;

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncSDevROI\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif




/*
** mitkIpFuncSDevROI
*/

ipFloat8_t mitkIpFuncSDevROI ( mitkIpPicDescriptor *pic_old,
                           ipUInt4_t       *pol_x,
                           ipUInt4_t       *pol_y,
                           ipUInt4_t       no_pts ) 
{
   ipFloat8_t      var;             /* variance  of greyvalues in ROI   */
   ipFloat8_t      s_dev;           /* standard dev.of greyvalues in ROI*/

   var = mitkIpFuncVarROI ( pic_old, pol_x, pol_y, no_pts );

   s_dev = sqrt ( var );

  return ( s_dev );

} 

#endif

