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

/*
 * ipFuncBorder
 *---------------------------------------------------------------------
 * DESCRIPTION
 *
 * FUNCTION DECLARATION
 *
 * PARAMETERS
 *
 * RETURN VALUES
 *
 * AUTHOR & DATE
 *  Antje Schroeder	27.07.95
 *
 * UPDATES
 * $Log$
 * Revision 1.4  2000/08/02 20:09:05  tobiask
 * Made some corrections in _BorderX.c
 *
 * Revision 1.3  2000/07/24 13:06:26  ivo
 * June 26, 2000
 * iw - function ipFuncSetTag added.
 *
 * July 8, 2000
 * iw - Visual C++ 6.0 SP3 compiler crashes at "static char *what..." lines.
 * Workaround: "lint" defined in project files for all configurations.
 *
 * July 11, 2000
 * iw - ipFunc.exe Scale crashed, if last parameter was missing. Corrected.
 *
 * Revision 1.2  2000/05/24 15:29:44  tobiask
 * Changed the installation paths of the package.
 *
 * Revision 1.3  2000/05/04 12:52:11  ivo
 * inserted BSD style license
 *
 * Revision 1.2  2000/02/18 14:58:09  ivo
 * Tags are now copied into newly allocated images.
 * Bugs fixed in ipFuncFrame, ipFuncRegGrow, _ipFuncBorderX and ipFuncHitMiss.
 *
 * Revision 1.1.1.1  2000/02/18  15:40:34  ivo
 * the dimension sizes n[i] that are not used must be set to zero,
 * otherwise the macro BORDER may crash. This is now done in the else-
 * part of "if ( pic_new == NULL )" (first two rows).
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* include files                                                                  */

#include "ipFuncP.h"

/* definition of macros                                                           */


#define BORDER( type, pic, size, value )                                           \
{                                                                                  \
  ipUInt4_t      i;                /* loop index                                */ \
  ipUInt4_t      anf[_mitkIpPicNDIM];  /*                                           */ \
  ipUInt4_t      ind[_mitkIpPicNDIM];  /* loop index vector                         */ \
  ipUInt4_t      off[_mitkIpPicNDIM];  /* used to calculate offset of image pixels  */ \
                                                                                   \
                                                                                   \
  for ( i = 0; i < pic->dim; i++ )                                                 \
    {                                                                              \
       anf[i] = 0;                                                                 \
       end[i] = pic->n[i];                                                         \
    }                                                                              \
  for ( i = pic->dim; i < _mitkIpPicNDIM; i++ )                                        \
    {                                                                              \
       anf[i] = 0;                                                                 \
       end[i] = 1;                                                                 \
       pic->n[i] = 1;                                                              \
    }                                                                              \
  for ( i = 0; i < pic->dim; i++ )                                                 \
    {                                                                              \
       anf[i] = pic->n[i] - edge[i];                                               \
       end[i] = edge[i];                                                           \
       for ( ind[7] = 0; ind[7] < end[7]; ind[7]++ )                               \
       {                                                                           \
         off[7] = size[7] * ind[7];                                                \
         for ( ind[6] = 0; ind[6] < end[6]; ind[6]++ )                             \
         {                                                                         \
           off[6] = size[6] * ind[6] + off[7];                                     \
           for ( ind[5] = 0; ind[5] < end[5]; ind[5]++ )                           \
           {                                                                       \
             off[5] = size[5] * ind[5] + off[6];                                   \
             for ( ind[4] = 0; ind[4] < end[4]; ind[4]++ )                         \
             {                                                                     \
               off[4] = size[4] * ind[4] + off[5];                                 \
               for ( ind[3] = 0; ind[3] < end[3]; ind[3]++ )                       \
               {                                                                   \
                 off[3] = size[3] * ind[3] + off[4];                               \
                 for ( ind[2] = 0; ind[2] < end[2]; ind[2]++ )                     \
                 {                                                                 \
                   off[2] = size[2] * ind[2] + off[3];                             \
                   for ( ind[1] = 0; ind[1] < end[1]; ind[1]++ )                   \
                   {                                                               \
                     off[1] = size[1] * ind[1] + off[2];                           \
                     for ( ind[0] = 0; ind[0] < end[0]; ind[0]++ )                 \
                     {                                                             \
                       (( type * ) pic_new->data )[off[1]] = ( type ) value;       \
                       off[1]++;                                                   \
                     }                                                             \
                   }                                                               \
                 }                                                                 \
               }                                                                   \
             }                                                                     \
           }                                                                       \
         }                                                                         \
       }                                                                           \
       for ( ind[7] = anf[7]; ind[7] < pic->n[7]; ind[7]++ )                       \
       {                                                                           \
         off[7] = size[7] * ind[7];                                                \
         for ( ind[6] = anf[6]; ind[6] < pic->n[6]; ind[6]++ )                     \
         {                                                                         \
           off[6] = size[6] * ind[6] + off[7];                                     \
           for ( ind[5] = anf[5]; ind[5] < pic->n[5]; ind[5]++ )                   \
           {                                                                       \
             off[5] = size[5] * ind[5] + off[6];                                   \
             for ( ind[4] = anf[4]; ind[4] < pic->n[4]; ind[4]++ )                 \
             {                                                                     \
               off[4] = size[4] * ind[4] + off[5];                                 \
               for ( ind[3] = anf[3]; ind[3] < pic->n[3]; ind[3]++ )               \
               {                                                                   \
                 off[3] = size[3] * ind[3] + off[4];                               \
                 for ( ind[2] = anf[2]; ind[2] < pic->n[2]; ind[2]++ )             \
                 {                                                                 \
                   off[2] = size[2] * ind[2] + off[3];                             \
                   for ( ind[1] = anf[1]; ind[1] < pic->n[1]; ind[1]++ )           \
                   {                                                               \
                     off[1] = size[1] * ind[1] + off[2];                           \
                     off[0] = off[1] + anf[0];                                     \
                     for ( ind[0] = anf[0]; ind[0] < pic->n[0]; ind[0]++ )         \
                     {                                                             \
                       (( type * ) pic_new->data )[off[0]] = ( type ) value;       \
                       off[0]++;                                                   \
                     }                                                             \
                   }                                                               \
                 }                                                                 \
               }                                                                   \
             }                                                                     \
           }                                                                       \
         }                                                                         \
       }                                                                           \
     end[i] = pic->n[i];                                                           \
     anf[i] = 0;                                                                   \
    }                                                                              \
}

/* ------------------------------------------------------------------------------ */
/*
*/
/* ------------------------------------------------------------------------------ */

ipPicDescriptor *_ipFuncBorderX ( ipPicDescriptor *pic_old,
                                  ipUInt4_t       *edge,
                                  ipFloat8_t      value )
{
  ipPicDescriptor *pic_new;            /* pointer to transformed image            */
  ipUInt4_t       i;                   /* loop index                              */
  ipUInt4_t      end[_mitkIpPicNDIM];      /* end of image                            */
  ipUInt4_t      size[_mitkIpPicNDIM];     /*                                         */
  ipFloat8_t     max_gv, min_gv;

  /* check whether data are correct                                               */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  pic_new = ipPicCopyHeader ( pic_old, NULL );
  if ( pic_new == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  else 
    {
       for ( i = pic_new->dim; i < _mitkIpPicNDIM; i++ ) 
         pic_new->n[i] = 0;

       for ( i = 0; i < pic_old->dim; i++ )
         pic_new->n[i] = pic_old->n[i] + 2 * edge[i];
       pic_new->data = calloc ( _ipPicElements ( pic_new ), pic_new->bpe/8 );
       if ( pic_new->data == NULL )
         {
            _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
            ipPicFree ( pic_new );
            return  ( mitkIpFuncERROR );
         }
    }


  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( ( value > max_gv ) ||  ( value < min_gv ) )
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
    

  /* initialize vectors and variables                                             */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_new->n[i-1];
  size[pic_new->dim] = 0;

  mitkIpPicFORALL_2 ( BORDER, pic_new, size, value )    

  return ( pic_new );
}
