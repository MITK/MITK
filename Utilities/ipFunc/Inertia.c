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
 * mitkIpFuncInertia
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  function to calculate the moments of inertia of a region of
 *  interest
 *
 * FUNCTION DECLARATION
 *  ipInt4_t mitkIpFuncInertia ( mitkIpPicDescriptor *pic_old,
 *                           ipFloat8_t     **eigen_vekt,
 *                           ipFloat8_t     **eigen_val )            
 *
 * PARAMETERS
 *  pic_old	- pointer to an image structure which contains the ROI
 *  eigen_vekt  - contains the axis of inertia (spaltenweise)
 *  eigen_val   - eigenvalues of the tensor of inertia
 *
 * RETURN VALUES
 *  mitkIpFuncOK        - no error occured
 *  mitkIpFuncERROR     - an error occured              
 *
 * UPDATES
 *   update of Manu's program to calculate axis of inertia. It's a
 *   function now, which can be used for images of different data
 *   types and for images up to 8 dimensions now
 *
 * AUTHOR & DATE
 *  Antje Schroeder	08.06.95
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
/* include Files                                                        */

#include "mitkIpFuncP.h"

#ifdef MESCHACH
#include <float.h>

/* definition of reflect-macro                                          */

#define GRAV( type, pic, index, s, s_diag, dist )                         \
{                                                                         \
  ipUInt4_t   i;                                                          \
  ipUInt4_t   no;                                                         \
  ipUInt4_t   offset_refl;                                                \
  ipInt4_t    n[_mitkIpPicNDIM];                                              \
                                                                          \
  for ( i = 0; i < pic_old->dim; i++ )                                    \
    n[i] = pic_old->n[i];                                                 \
                                                                          \
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )                           \
    n[i] = 1;                                                             \
                                                                          \
  offset_refl = 0;                                                        \
  for ( index[7] = 0; index[7] < n[7]; index[7]++ )                       \
                                                                          \
    for ( index[6] = 0; index[6] < n[6]; index[6]++ )                     \
                                                                          \
      for ( index[5] = 0; index[5] < n[5]; index[5]++ )                   \
                                                                          \
        for ( index[4] = 0; index[4] < n[4]; index[4]++ )                 \
                                                                          \
          for ( index[3] = 0; index[3] < n[3]; index[3]++ )               \
                                                                          \
            for ( index[2] = 0; index[2] < n[2]; index[2]++ )             \
                                                                          \
              for ( index[1] = 0; index[1] < n[1]; index[1]++ )           \
                                                                          \
                for ( index[0] = 0; index[0] < n[0]; index[0]++ )         \
                {                                                         \
                  if ( (( type * )pic->data )[offset_refl] )              \
                    {                                                     \
                       for ( i = 0; i < pic->dim; i++ )                   \
                          {                                               \
                             dist[i]   = index[i] - gravity[i];           \
                             s_diag[i] = s_diag[i] + dist[i] * dist[i];   \
                          }                                               \
                                                                          \
                        for ( i = 0; i < pic->dim; i++ )                  \
                          for ( j = 0; j < pic->dim; j++ )                \
                             s[i*pic->dim+j] = s[i*pic->dim+j] - dist[i] * dist[j];\
                    }                                                     \
                  offset_refl++;                                          \
                }                                                         \
                                                                          \
}



/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipInt4_t mitkIpFuncInertia ( mitkIpPicDescriptor *pic_old,
                         ipFloat8_t     **eigen_vekt,
                         ipFloat8_t     **eigen_val )            
{

  ipUInt4_t       index_vect[_mitkIpPicNDIM]; /* loopindex-vector           */
  ipInt4_t        n[_mitkIpPicNDIM];          /* number of pixels in each   */
                                          /* dimension                  */
  ipUInt4_t       i, j;                   /* loop index                 */
  ipFloat8_t      *gravity;               /* center of gravity          */
  ipFloat8_t      *help_vekt;             /* pointer to eigen_vekt      */
  ipFloat8_t      *help_val;              /* pointer to eigen_val       */
  MAT             *ev;                    /* eigenvector                */
  MAT             *tt;                    /* tensor of inertia          */
  VEC             *ew;                    /* eigenvalue                 */
  ipFloat8_t      *s, *s_diag, *dist;     /* used to calculate tt       */

  /* check data  */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
 
  /* initialisation of vectors  */

  for ( i = 0; i < pic_old->dim; i++ )
    n[i] = pic_old->n[i];
  
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;

  for ( i = 0; i < _mitkIpPicNDIM; i++ )
    index_vect[i] = 0;

  /* memory allocation */

  gravity = ( ipFloat8_t * ) malloc ( pic_old->dim * sizeof ( ipFloat8_t ) );
  if ( gravity == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  dist    = ( ipFloat8_t * ) malloc ( pic_old->dim * sizeof ( ipFloat8_t ) );
  if ( dist == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       free ( gravity );
       return ( mitkIpFuncERROR );
    }
 
  s_diag  = ( ipFloat8_t * ) malloc ( pic_old->dim * sizeof ( ipFloat8_t ) );
  if ( s_diag == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       free ( gravity );
       free ( dist );    
       return ( mitkIpFuncERROR );
    }
 
  s = ( ipFloat8_t * ) malloc ( pic_old->dim * pic_old->dim * sizeof ( ipFloat8_t ) );
  if ( s == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       free ( gravity );
       free ( dist );   
       free ( s_diag );
       return ( mitkIpFuncERROR );
    }
 
  tt = m_get ( pic_old->dim, pic_old->dim );
  if ( tt == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       free ( gravity );
       free ( dist );   
       free ( s_diag );
       free ( s );
       return ( mitkIpFuncERROR );
    }

  ev = m_get ( pic_old->dim, pic_old->dim );
  if ( ev == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       free ( gravity );
       free ( dist );   
       free ( s_diag );
       free ( s );
       M_FREE ( tt );
       return ( mitkIpFuncERROR );
    }

  ew = v_get ( pic_old->dim-1 );
  if ( ew == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       free ( gravity );
       free ( dist );   
       free ( s_diag );
       free ( s );
       M_FREE ( tt );
       M_FREE ( ev );
       return ( mitkIpFuncERROR );
    }

  /* calculate center of gravity  */

  gravity = mitkIpFuncGrav ( pic_old );

  /* Initialization of vectors */

  for ( i = 0; i < pic_old->dim; i++ )
     {
        s_diag[i] = 0.;
        dist[i]   = 0.;
        for ( j = 0; j < pic_old->dim; j++ )
          s[i*pic_old->dim+j] = 0.;
     }

  /* preparation for calculating the tensor of inertia */

  mitkIpPicFORALL_4 ( GRAV, pic_old, index_vect, s, s_diag, dist ) 

  /* calculate tensor of inertia  */

  for ( i = 0; i < pic_old->dim; i++ )
    {
       tt->me[i][i] = 0.;
       for ( j = 0; j < pic_old->dim; j++ )
         {
            if ( i < j )
               tt->me[i][j] = s[i*pic_old->dim+j];
            else if ( j < i )
               tt->me[i][j] = s[j*pic_old->dim+i];
            if ( i != j ) 
               tt->me[i][i] = tt->me[i][i] + s_diag[j];
         }
    }

  /* calculate eigenvectors and eigenvalues of the tensor of inertia */

  ew = symmeig ( tt, ev, ew );

  *eigen_vekt = ( ipFloat8_t * ) malloc ( pic_old->dim * pic_old->dim * sizeof ( ipFloat8_t ) );
  help_vekt   = *eigen_vekt;
  *eigen_val  = ( ipFloat8_t * ) malloc ( pic_old->dim * sizeof ( ipFloat8_t ) );
  help_val    = *eigen_val;

  for ( i = 0; i < pic_old->dim; i++ )
    {
      help_val[i] = ew->ve[i];
      for ( j = 0; j < pic_old->dim; j++ )
        help_vekt[i*pic_old->dim+j] = ev->me[i][j];
    }

  M_FREE ( tt );
  M_FREE ( ev );
  V_FREE ( ew );
  free ( s );
  free ( dist );
  free ( s_diag );
  free ( gravity );

  return mitkIpFuncOK;   
}
#endif
