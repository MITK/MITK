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
*  this function equalizes a greyvalue histogram        
*/


/**  this function equalizes a greyvalue histogram                     
 *
 *  @param pic_old    pointer to the image that should be equalized
 *  @param kind      intervall of transformation
 *  @arg @c ipFuncMinMax       (extreme greyvalues)
 *  @arg @c ipFuncTotal        (max. and min. possible       
 *                                           greyvalues) 
 *  @param pic_return  memory used to store return image ( if pic_return == NULL
 *                new memory is allocated )
 *
 * @return pointer to the egalized image 
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"   

ipPicDescriptor *ipFuncEqual ( ipPicDescriptor *pic_old,
                               ipFuncFlagI_t   kind,
                               ipPicDescriptor *pic_return ) ;

#ifndef DOXYGEN_IGNORE


#ifndef lint
  static char *what = { "@(#)ipFuncEqual\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif




/* definition of EQUAL-macro                                            */

#define EQUAL( type, pic, pic_new, kind, factor, hist, help, size_hist ) \
{                                                                        \
  type       *hist_cp;                                                   \
  ipUInt4_t  i, no_elem;                                                 \
  ipUInt4_t  index;                                                      \
  ipFloat8_t a, b;                                                       \
  ipFloat8_t min, max;   /* intervall for transformation           */    \
                                                                         \
  /* allocate memory for the transformed histogram                 */    \
                                                                         \
  hist_cp =  calloc( size_hist + 1, sizeof( type ) );                    \
  if ( hist_cp == NULL )                                                 \
    {                                                                    \
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );                            \
       return ( NULL );                                                  \
    }                                                                    \
                                                                         \
  /* cumulative greylevel histogram                                */    \
                                                                         \
  for ( i = 1; i <= size_hist; i++ )                                     \
    {                                                                    \
     hist[i] =  hist[i] + hist[i-1];                                     \
    }                                                                    \
                                                                         \
  /* check wether linerisation in [min_gv, max_gv] or in [min_max]  */   \
                                                                         \
  if ( kind == ipFuncMinMax )                                            \
    {                                                                    \
      if ( ipFuncExtr ( pic_old, &min, &max ) != mitkIpFuncOK )                  \
        return ( mitkIpFuncERROR );                                              \
    }                                                                    \
  else if ( kind == ipFuncTotal )                                        \
    {                                                                    \
      max = max_gv;                                                      \
      min = min_gv;                                                      \
    }                                                                    \
  else                                                                   \
    {                                                                    \
      _ipFuncSetErrno ( mitkIpFuncFLAG_ERROR );                              \
      free ( hist_cp );                                                  \
      free ( hist );                                                     \
      return ( mitkIpFuncERROR );                                                \
    }                                                                    \
                                                                         \
  /* transformation of histogram                                    */   \
                                                                         \
  a = ( ipFloat8_t ) _ipPicElements ( pic ) / ( max - min );             \
  b = - a * min;                                                         \
  for ( i = 0; i <= size_hist; i++ )                                     \
     hist_cp[i] =  ( type ) ( ( ( ipFloat8_t )hist[i] - b ) / a );       \
                                                                         \
  /* transform greyvalues                                           */   \
                                                                         \
  no_elem = _ipPicElements ( pic );                                      \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
       index = ( ipUInt4_t )                                             \
               ( factor * ((( type * ) pic->data )[i] + ( type )help ) );\
       ( ( type * ) pic_new->data )[i] = ( type ) hist_cp[index];        \
    }                                                                    \
  free ( hist_cp );                                                      \
} 
               
 

/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncEqual ( ipPicDescriptor *pic_old,
                               ipFuncFlagI_t   kind,
                               ipPicDescriptor *pic_return ) 
{

  ipPicDescriptor *pic_new;  /* inverted picture                        */
  ipFloat8_t      max_gv;    /* max. possible greyvalue                 */
  ipFloat8_t      min_gv;    /* min. possible greyvalue                 */
  ipUInt4_t       *hist;     /* greylevel histogram                     */
  ipUInt4_t       size_hist; /* no. of elements in histogram            */
  ipFloat4_t      factor;  
  ipFloat8_t      help;

  /* check data                                                         */
  
  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* create a new picture, copy the header, allocate memory             */

  pic_new = _ipFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );     
  if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* calculate max. and min. possible greyvalues                        */

  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK ) 
    {
       ipPicFree ( pic_new );
       return ( NULL );
    }

  /* calculate greylevel histogram                                      */

  ipFuncHist ( pic_old, min_gv, max_gv, &hist, &size_hist ); 
  if ( hist == 0 ) 
    {
       ipPicFree ( pic_new );
       return ( NULL );
    }

  /* help variables to calculate histogram                              */

  help = fabs ( min_gv );

  if ( pic_old->type == ipPicFloat ) 
    factor = 1000.;                              
  else if ( pic_old->type == ipPicInt || pic_old->type == ipPicUInt )
    factor = 1.;
  else 
    {
       ipPicFree ( pic_new );
       free ( hist );
       _ipFuncSetErrno ( mitkIpFuncTYPE_ERROR );
       return ( NULL );
    }

  /* macro to change image (for all data types)                         */

  mitkIpPicFORALL_6 ( EQUAL, pic_old, pic_new, kind, factor, hist, help, size_hist );

  free ( hist );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  


  return ( pic_new );
}
#endif

