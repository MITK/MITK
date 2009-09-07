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
 *   this function allocates the memory for a new image depending on the
 *   parameter pic_return. If pic_return is equal NULL, new memory is     
 *   allocated otherwise it is reused.
 */

/**  this function allocates the memory for a new image depending on the
 *   parameter pic_return. If pic_return is equal NULL, new memory is     
 *   allocated otherwise it is reused.
 *
 *   @param pic_old     pointer to original image
 *   @param pic_return  pointer to the memory which should be reused
 *   @param over_write  tells whether the memory could be reused
 *   @arg @c ipTrue  => memory could be reused
 *   @arg @c ipFalse => new memory must be allocated
 *                 
 *   @return  pointer to the memory for transformed image
 *
 * AUTHOR & DATE
 */

/* include files                                                         */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *_mitkIpFuncMalloc ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return,
                                ipBool_t        over_write );

#ifndef DOXYGEN_IGNORE


#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/*
** mitkIpFuncMalloc
*/ 

mitkIpPicDescriptor *_mitkIpFuncMalloc ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return,
                                ipBool_t        over_write )
{
   ipBool_t   okay=ipTrue;
   ipUInt4_t  i;

   if ( pic_return == NULL || ( !over_write ) )
     {
        pic_return = mitkIpPicCopyHeader ( pic_old, 0 );
        if ( pic_return == NULL )
          {
             _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
             return ( mitkIpFuncERROR );
          }
        pic_return->data = calloc ( _mitkIpPicElements ( pic_old ), pic_old->bpe/8 );
        if ( pic_return->data == NULL )
          {
             mitkIpPicFree ( pic_return );
             _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
             return ( mitkIpFuncERROR );
          }
     } 
   else 
     {
        if ( pic_old->type != pic_return->type ) okay = ipFalse;
        if ( pic_old->bpe  != pic_return->bpe  ) okay = ipFalse;
        if ( pic_old->dim  != pic_return->dim  ) okay = ipFalse;
        for ( i = 0; i < pic_old->dim; i++ )
          if ( pic_old->n[i] != pic_return->n[i] ) okay = ipFalse;
     }

   if ( !okay )
     {
        mitkIpPicFree ( pic_return );
        pic_return = mitkIpPicCopyHeader ( pic_old, 0 );
        if ( pic_return == NULL )
          {
             _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
             return ( mitkIpFuncERROR );
          }
        pic_return->data = calloc ( _mitkIpPicElements ( pic_old ), pic_old->bpe/8 );
        if ( pic_return->data == NULL )
          {
             _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
             return ( mitkIpFuncERROR );
          }
     } 
   
   return ( pic_return );
}
#endif

