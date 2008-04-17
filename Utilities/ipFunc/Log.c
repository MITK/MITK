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
 *  this functions transforms the greyvalues of an image with a lg-function
 *  (y = a lg (x+1))
 */

/**  this functions transforms the greyvalues of an image with a lg-function
 *  (y = a lg (x+1))
 *  @param pic_old  pointer to original image 
 *
 *  @return  pointer to transformed image
 *
 * AUTHOR & DATE
 */

/* include files                                                            */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncLog ( ipPicDescriptor *pic_old );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncLog\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/* definition of macros                                                     */

#define LOG( type, pic, max_gv )                                             \
{                                                                            \
  ipUInt4_t    i;           /* loopindex                                   */\
  ipUInt4_t    no_elem;     /*                                             */\
  ipFloat8_t   a;           /* factor to calculate the logaritm            */\
                                                                             \
  a = ( max_gv ) / log10 ( max_gv + 1 );                                     \
                                                                             \
  no_elem = _ipPicElements ( pic );                                          \
  for ( i = 0; i < no_elem; i++ )                                            \
    (( type * )pic_new->data )[i] =                                          \
        ( type ) ( a * log10 ( 1. + (( type * )pic->data )[i] ) );           \
}                                           

/* ------------------------------------------------------------------------ */
/*
*/
/* ------------------------------------------------------------------------ */

ipPicDescriptor *ipFuncLog ( ipPicDescriptor *pic_old )
{
  ipFloat8_t       min_gv, max_gv;  /* max and min posiible greyvalues      */
  ipPicDescriptor  *pic_new;        /* pointer to transformed image         */
  ipUInt4_t        i;               /* loop index                           */
  ipUInt4_t        no_elem;         /* loop index                           */

  /* check whether data are correct                                         */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues                            */

  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
    return ( mitkIpFuncERROR );

  /* check datatype of image                                                */

  no_elem = _ipPicElements ( pic_old );
  if ( pic_old->type == ipPicUInt )
    {
       pic_new = ipPicCopyHeader ( pic_old, NULL );
       pic_new->data = malloc ( _ipPicSize ( pic_old ) );
              
       if ( ( pic_new != NULL ) && ( pic_new->data != NULL ) )
         {
            mitkIpPicFORALL_1 ( LOG, pic_old, max_gv );
         }
       else 
         {
            _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
            return ( mitkIpFuncERROR );
         }
    }

  /* copy integer image to usigned integer image to avoid negative greyvalues*/

  else if ( pic_old->type == ipPicInt )
    {
       pic_new = ipPicNew ();
       if ( pic_new != NULL )
         {
            pic_new->type  = ipPicUInt;
            pic_new->bpe   = pic_old->bpe;
            pic_new->dim   = pic_old->dim;
            for ( i = 0; i < pic_new->dim; i++ ) pic_new->n[i] = pic_old->n[i];
            pic_new->data = malloc ( _ipPicSize ( pic_new ) );
            if ( pic_new->data == NULL )
              {
                 ipPicFree ( pic_new );
                 _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
                 return ( mitkIpFuncERROR );
              }

            /* transform integer image to unsigned integer image           */

            if ( pic_new->bpe == 8 )
              { 
                for ( i = 0; i < no_elem; i++ )                        
                  (( ipUInt1_t * )pic_new->data )[i] =
                     (( ipInt1_t * )pic_old->data )[i] - min_gv;
              } 

            else if ( pic_new->bpe == 16 )
              { 
                for ( i = 0; i < no_elem; i++ )                        
                  (( ipUInt2_t * )pic_new->data )[i] =
                    (( ipInt2_t * )pic_old->data )[i] - min_gv;
              } 

            else if ( pic_new->bpe == 32 )
              { 
                for ( i = 0; i < no_elem; i++ )                        
                  (( ipUInt4_t * )pic_new->data )[i] =
                    (( ipInt4_t * )pic_old->data )[i] - min_gv;
              } 

            else
              {
                  ipPicFree ( pic_new );
                  _ipFuncSetErrno ( mitkIpFuncTYPE_ERROR );
                  return ( mitkIpFuncERROR );
              }

            max_gv = max_gv - min_gv;

            mitkIpPicFORALL_1 ( LOG, pic_new, max_gv );

            /* transform unsigned integer image to an integer image           */

            pic_new->type = ipPicInt;
            if ( pic_new->bpe == 8 )
              { 
                for ( i = 0; i < no_elem; i++ );                       
                  (( ipInt1_t * )pic_new->data )[i] =
                    (( ipUInt1_t * )pic_new->data )[i] + min_gv;
              } 

            else if ( pic_new->bpe == 16 )
              { 
                for ( i = 0; i < no_elem; i++ )                        
                  (( ipInt2_t * )pic_new->data )[i] =
                    (( ipUInt2_t * )pic_new->data )[i] + min_gv;
              } 

            else if ( pic_new->bpe == 32 )
              { 
                for ( i = 0; i < no_elem; i++ )                        
                  (( ipInt4_t * )pic_new->data )[i] =
                    (( ipUInt4_t * )pic_new->data )[i] + min_gv;
              } 

         }

     }
   else 
     {
        _ipFuncSetErrno ( mitkIpFuncTYPE_ERROR );
        return ( mitkIpFuncERROR );
     }

   return ( pic_new );
}
#endif

