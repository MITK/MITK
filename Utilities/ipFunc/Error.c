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
 *   this functions checks, whether the information of the image header
 *   are correct
 */

 /** @briefs checks, whether the information of the image header
 *   are correct
 *
 *   @param pic   pointer to the image           
 *
 *   @return mitkIpFuncOK     - if data are correct              
 *   @return mitkIpFuncERROR  - if data aren't correct
 *
 * AUTHOR & DATE
 */

/* include files                                                      */

# include "mitkIpFuncP.h"

ipUInt4_t _mitkIpFuncError ( mitkIpPicDescriptor *pic );


#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/*-------------------------------------------------------------------*/
/*
*/
/*-------------------------------------------------------------------*/

ipUInt4_t _mitkIpFuncError ( mitkIpPicDescriptor *pic )
{
  ipUInt4_t   i;                      /* loop index                  */

  /* is image existing ?                                             */

  if ( !pic ) 
    {
      _mitkIpFuncSetErrno ( mitkIpFuncNOPIC_ERROR );
      return ( mitkIpFuncERROR );
    }

  /* is dimension correct ?                                          */

  if ( pic->dim < 1 || pic->dim > _mitkIpPicNDIM )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* is size correct ?                                               */
  
  for ( i = 0; i < pic->dim; i++ )
    if ( pic->n[i] < 1 )
      {
         _mitkIpFuncSetErrno ( mitkIpFuncSIZE_ERROR );
         return ( mitkIpFuncERROR );
      }
  for ( i = pic->dim; i < _mitkIpPicNDIM; i++ ) 
    pic->n[i] = 0;
   
  /* is image data type correct ?                                     */

  if ( pic->type == mitkIpPicInt || pic->type == mitkIpPicUInt )
    {
       if ( pic->bpe != 8 && pic->bpe != 16 && pic->bpe != 32 )
         {
           _mitkIpFuncSetErrno ( mitkIpFuncTYPE_ERROR );
           return ( mitkIpFuncERROR );
         }
    }
  else if ( pic->type == mitkIpPicFloat )
    {
       if ( pic->bpe != 32 && pic->bpe != 64 )
         {
            _mitkIpFuncSetErrno ( mitkIpFuncTYPE_ERROR );
            return ( mitkIpFuncERROR );
         }
    }
  else 
    {
      _mitkIpFuncSetErrno ( mitkIpFuncTYPE_ERROR );
      return ( mitkIpFuncERROR );
    }
  
  return ( mitkIpFuncOK );
}
#endif

