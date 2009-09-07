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
 *  this function calculates the greyvalue histogram of an image
 */

/**  this function calculates the greyvalue histogram of an image
 *
 *  @param pic_old  pointer to the image that should be inverted
 *  @param min_gv   min. greyvalue in histogram; if min_gv==max_gv==0
 *					the min/max greyvalues are calculated
 *  @param max_gv   max. greyvalue in histogram
 *  @param hist     pointer to the greyvalue histogram ( return value )
 *  @param size_hist   pointer to the number of elements in the histogram
 *                ( result value )
 *
 *  @return @arg @c mitkIpFuncERROR     - if an error occured
 *  @return @arg @c mitkIpFuncOK        - if no error occured
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   

ipInt4_t  mitkIpFuncHisto( mitkIpPicDescriptor *pic_old, 
                       ipFloat8_t      *min_gv,
                       ipFloat8_t      *max_gv,
                       ipUInt4_t       **hist,
                       ipUInt4_t       *size_hist ) ;

#ifndef DOXYGEN_IGNORE
 
/* definition of HIST-macro                                             */

#define HIST( type, pic, help, factor )                                  \
{                                                                        \
  ipUInt4_t  i;                                                          \
  ipUInt4_t  no_elem;                                                    \
  ipUInt4_t  index;                                                      \
                                                                         \
  /* calculate histogram                                             */  \
                                                                         \
  no_elem =  _mitkIpPicElements ( pic );                                     \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
       index = ( ipUInt4_t )                                             \
               ( factor * ( (( type * ) pic->data)[i] - ( type )*min_gv ) );\
       if ( ( index <= *size_hist ) )                                    \
          ( hist_help[index] )++;                                        \
    }                                                                    \
} 
               
 

/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipInt4_t  mitkIpFuncHisto( mitkIpPicDescriptor *pic_old, 
                       ipFloat8_t      *min_gv,
                       ipFloat8_t      *max_gv,
                       ipUInt4_t       **hist,
                       ipUInt4_t       *size_hist ) 
{
  ipUInt4_t       *hist_help;
  ipFloat8_t      help;            /* absolute of min_gv                */
  ipFloat8_t      min, max;        /* extreme greyvalues                */
  ipUInt4_t       factor;          /* factor to calculate histogram of  */
                                   /* float images                      */

  /* check whether data are correct                                     */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( *min_gv == 0 && *max_gv == 0 )
    {
       if ( mitkIpFuncExtr ( pic_old, min_gv, max_gv ) != mitkIpFuncOK ) 
	{
	printf("ipFunc: Probleme mit dem eingegebenen Intervall\n");
	return ( mitkIpFuncERROR );
	}
    }
  else 
    {
       if ( *min_gv > *max_gv ) 
         {
            _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
	    printf("ipFunc: Probleme mit dem eingegebenen Intervall\n");
            return ( mitkIpFuncERROR );
         }
       if ( mitkIpFuncExtr ( pic_old, &min, &max ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
       if ( *min_gv < min || *max_gv > max )
         {
	    printf("ipFunc: Probleme mit dem von ipFunc Extrema berechnete Intervall\n");
	    printf("ipFunc: Von Extrema: %f %f \n", *min_gv, *max_gv);
	   /*
            _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
            return ( mitkIpFuncERROR );
	   */
         }
    }

  /* calculate variables which are necessary to calculate histogram     */
  /* index                                                              */

  help = fabs ( *min_gv );

  if ( pic_old->type == mitkIpPicInt || pic_old->type == mitkIpPicUInt )
    factor = 1;
  else if ( pic_old->type == mitkIpPicFloat )
    factor = 1000;
  else 
    {
       printf("ipFunc: Probleme mit dem von ipFunc Extrema berechnete Intervall\n");
       _mitkIpFuncSetErrno (mitkIpFuncTYPE_ERROR );
       return ( mitkIpFuncERROR );
    }
  
  /* allocate memory                                                    */

  *size_hist = 1 + factor * (ipUInt4_t ) ( ( ipInt4_t )  *max_gv - ( ipInt4_t ) *min_gv );

/*hist  = ( ipUInt4_t ** ) malloc ( sizeof ( ipUInt4_t * ) );*/
  *hist = ( ipUInt4_t * )  calloc ( *size_hist+1, sizeof ( ipUInt4_t ) );
  hist_help = *hist;
  if ( hist_help == NULL ) 
    {
       printf("ipFunc: Probleme mit dem Allokieren von Platz\n");
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );                    
       return ( mitkIpFuncERROR );
    }

  /* macro to calculate the histogram                                   */

  mitkIpPicFORALL_2 ( HIST, pic_old, help, factor );

  return ( mitkIpFuncOK );
}
#endif

