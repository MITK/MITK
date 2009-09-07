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
 * _mitkIpFuncBox3d 
 *---------------------------------------------------------------------
 * DESCRIPTION
 *   This Function calulates the bounding box                               
 *
 * FUNCTION DECLARATION
 *
 * PARAMETERS
 *   pic_old  - binary image: contains the region of interest
 *   beg      - pointer to left upper corner
 *   end      - pointer to right lower corner
 *
 * RETURN VALUES
 *   mitkIpFuncOK     - no error occured
 *   mitkIpFuncERROR  - error occured (description of error in mitkIpFuncErrno)
 *
 * UPDATES
 *   this version could only be used for 3D images, in later version 
 *   it should be possible to use it for every dimension
 *
 * AUTHOR & DATE
 *  Antje Schroeder	08.01.97
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
/* include Files                                                        */

#include "mitkIpFuncP.h"

/* definition of Box-macro                                              */

#define BOX3( type, pic, help_beg, help_end, n, size )                    \
{                                                                         \
  ipUInt4_t   offset1, offset2;                                           \
                                                                          \
  for ( l = 0; l < pic->dim; l++ )                                        \
   {                                                                      \
      n[l] = pic->n[l];                                                   \
   }                                                                      \
                                                                          \
  for ( l = 0; l < pic->dim; l++ )                                        \
    {                                                                     \
       stop     = ipFalse;                                                \
       stop_beg = ipFalse;                                                \
       stop_end = ipFalse;                                                \
       k = 0;                                                             \
       while ( k < n[order[2+pic_old->dim*l]] && ( !stop ) )              \
         {                                                                \
            j = 0;                                                        \
            while ( j < n[order[1+pic_old->dim*l]] && ( !stop ) )         \
              {                                                           \
                 i = 0;                                                   \
                 while ( i < n[order[pic_old->dim*l]] && ( !stop ) )      \
                   {                                                      \
                      offset1 = i * size[order[0+pic_old->dim*l]] +       \
                                j * size[order[1+pic_old->dim*l]] +       \
                                k * size[order[2+pic_old->dim*l]];        \
                                                                          \
                      offset2 = ( n[order[0+pic_old->dim*l]] - i - 1 ) *  \
                                     size[order[0+pic_old->dim*l]] +      \
                                ( n[order[1+pic_old->dim*l]] - j - 1 ) *  \
                                     size[order[1+pic_old->dim*l]] +      \
                                ( n[order[2+pic_old->dim*l]] - k - 1 ) *  \
                                     size[order[2+pic_old->dim*l]];       \
                                                                          \
                      if ( ( stop_beg == ipFalse ) &&                     \
                           ( ( type * )pic->data )[offset1] )             \
                        {                                                 \
                           help_beg[order[2+pic->dim*l]] = k;             \
                           stop_beg = ipTrue;                             \
                        }                                                 \
                      if ( ( stop_end == ipFalse ) &&                     \
                           ( ( type * )pic->data )[offset2] )             \
                        {                                                 \
                           help_end[order[2+pic->dim*l]] =                \
                                  n[order[2+pic->dim*l]] - k;             \
                           stop_end = ipTrue;                             \
                        }                                                 \
                      if ( stop_beg && stop_end ) stop = ipTrue;          \
                      i++;                                                \
                   }                                                      \
                 j++;                                                     \
              }                                                           \
            k++;                                                          \
         }                                                                \
    }                                                                     \
                                                                          \
}



/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipUInt4_t _mitkIpFuncBox3d ( mitkIpPicDescriptor *pic_old,
                         ipUInt4_t       **beg,
                         ipUInt4_t       **end )            
{

  ipUInt4_t       n[_mitkIpPicNDIM];          /* number of pixels in each   */
                                          /* dimension                  */
  ipUInt4_t       size[_mitkIpPicNDIM];       /* size of each direction     */
  ipUInt4_t       i, j, k, l;             /* loop index                 */
  ipUInt4_t       order[] = { 0, 1, 2,    /* order of indizes for each  */
                              2, 1, 0,    /* direction                  */
                              0, 2, 1 };
  ipBool_t        stop, stop_beg, stop_end;/* finish calculation?       */
  ipUInt4_t       *help_beg, *help_end;

  /* check data                                                         */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
   
  if ( pic_old->dim > 3 )   /* just in for the first version */
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  /* initialisation of vectors                                          */

  size[0] = 1;
  n[0]    = pic_old->n[0];
  for ( i = 1; i < pic_old->dim; i++ )
    {
       n[i] = pic_old->n[i];
       size[i] = size[i-1] * pic_old->n[i-1];
    }
  
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;

  /* allocation of result vectors                                       */

  *beg = ( ipUInt4_t * ) malloc ( pic_old->dim * sizeof ( ipUInt4_t ) );
  help_beg = *beg;
  *end = ( ipUInt4_t * ) malloc ( pic_old->dim * sizeof ( ipUInt4_t ) );
  help_end = *end;

  mitkIpPicFORALL_4 ( BOX3, pic_old, help_beg, help_end, n, size );             

  return ( mitkIpFuncOK );
}
