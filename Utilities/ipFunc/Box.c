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
 *  This Function calulates the bounding box
 */


/** This Function calulates the bounding box                               
 *
 *  @param pic_old binary image: contains the region of interest
 *  @param  beg pointer to left upper corner
 *  @param  end pointer to right lower corner
 *
 *  @return @arg @c mitkIpFuncOK no error occured
 *  @arg @c mitkIpFuncERROR error occured (description of error in mitkIpFuncErrno)
 *
 * @warning  this version could only be used for 2D and 3D images, in  
 *            later version it should be possible to use it for every dimension
 *
 * AUTHOR & DATE
 */

#include "mitkIpFuncP.h"

ipUInt4_t mitkIpFuncBox ( mitkIpPicDescriptor *pic_old,
                      ipUInt4_t       **beg,
                      ipUInt4_t       **end )       ;

#ifndef DOXYGEN_IGNORE

ipUInt4_t mitkIpFuncBox ( mitkIpPicDescriptor *pic_old,
                      ipUInt4_t       **beg,
                      ipUInt4_t       **end )            
{

  /* check data                                                         */

  if ( pic_old->dim == 2 )
    {
       _mitkIpFuncBox2d ( pic_old, beg, end );
       if ( mitkIpFuncErrno > mitkIpFuncOK )
          return ( mitkIpFuncERROR );
    }
  else if ( pic_old->dim == 3 )
    {
       _mitkIpFuncBox3d ( pic_old, beg, end );
       if ( mitkIpFuncErrno > mitkIpFuncOK )
          return ( mitkIpFuncERROR );
    }
  else 
    { 
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  return ( mitkIpFuncOK );
}

#endif

