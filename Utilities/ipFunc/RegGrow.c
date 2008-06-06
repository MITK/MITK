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
 *  this function performs a region growing by mean and standard deviation.
 */

/** @brief performs a region growing by mean and standard deviation.
 *
 *  The growing criteria is, that the new pixels greyvalue must be in the       
 *  standard deviation of the seed region.
 *  After calculating the mean and standard deviation of the seed region, all
 *  pixels of this region are investigated whether they belong to a region  
 *  or not. All accepted pixels are marked with a region_label. In the next 
 *  step the neigbours of all accepted pixels are investigated whether they
 *  belong to the region or to the border or to none of them. At the end all 
 *  pixels which belong to the boder (which contains the first pixel of the
 *  outside region) are marked.
 *
 *  @param pic_old       pointer to original image
 *  @param dim_seed      dimension of seed region
 *  @param beg_seed      array with start pixels of seed_region
 *  @param end_seed      array with end pixels of seed_region
 *  @param border_label  greyvalue for border pixels
 *  @param region_label  greyvalue for pixels in region
 *  @param std_fact      factor ( to multiply standard deviation with )
 *  @param kind          tells wether region or border should be shown
 *                    0 : border of region is shown
 *                    1 : region is shown
 *
 * @return pointer to new image
 *
 * USES
 *  function _ipFuncError - check image data
 *
 * AUTHOR & DATE
 * UPDATES
 *
 * $Log$
 * Revision 1.2  2000/05/24 15:29:43  tobiask
 * Changed the installation paths of the package.
 *
 * Revision 1.4  2000/05/04 12:52:10  ivo
 * inserted BSD style license
 *
 * Revision 1.3  2000/03/06 17:02:44  ivo
 * ipFunc now with doxygen documentation.
 *
 * Revision 1.2  2000/02/18 14:58:06  ivo
 * Tags are now copied into newly allocated images.
 * Bugs fixed in ipFuncFrame, ipFuncRegGrow, _ipFuncBorderX and ipFuncHitMiss.
 *
 * Revision 1.1.1.1  2000/02/18  15:22:50  ivo
 * in macro REGGROW line if(diff<std_win) changed to if ( diff <= std_win ),
 * because otherwise a standard deviation of zero will result in an emty
 * image.
 *
 */

/* include files                                                              */
 
#include "ipFuncP.h"

ipPicDescriptor *ipFuncRegGrow ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       dim_seed,
                                 ipUInt4_t       *beg_seed,
                                 ipUInt4_t       *end_seed,
                                 ipUInt4_t       border_label,
                                 ipUInt4_t       region_label,
                                 ipFloat8_t      std_fact,  
                                 ipUInt4_t       kind ) ;

#ifndef DOXYGEN_IGNORE


#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* include files                                                              */
 
#include "ipFuncP.h"

/* definition of macros                                                       */

#define STAT( type, pic_old, pic_new, mean, std_win, size_seed, beg, end )     \
{                                                                              \
  ipInt4_t     ind[_mitkIpPicNDIM];    /* loop index vector                     */ \
  ipInt4_t     off[_mitkIpPicNDIM];    /* used to calculate offset vector       */ \
  ipFloat8_t   sum;                /* sum of greyvalues in seed region      */ \
  ipFloat8_t   saq;                /* sum of deviation squares              */ \
  ipFloat8_t   diff;               /* used to calculate saq                 */ \
  ipFloat8_t   std_dev;            /* standard deviation                    */ \
                                                                               \
                                                                               \
  /* calculate mean                                                         */ \
                                                                               \
  sum = 0.;                                                                    \
  for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )                           \
  {                                                                            \
    off[7] = size[7] * ind[7];                                                 \
    for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )                         \
    {                                                                          \
      off[6] = size[6] * ind[6] + off[7];                                      \
      for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )                       \
      {                                                                        \
        off[5] = size[5] * ind[5] + off[6];                                    \
        for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )                     \
        {                                                                      \
          off[4] = size[4] * ind[4] + off[5];                                  \
          for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )                   \
          {                                                                    \
            off[3] = size[3] * ind[3] + off[4];                                \
            for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )                 \
            {                                                                  \
              off[2] = size[2] * ind[2] + off[3];                              \
              for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )               \
              {                                                                \
                off[1] = size[1] * ind[1] + off[2];                            \
                off[0] = off[1] + beg[0];                                      \
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )             \
                {                                                              \
                   sum = sum +                                                 \
                         ( ipFloat8_t ) (( type * ) pic_old->data) [off[0]];   \
                   off[0]++;                                                   \
                }                                                              \
              }                                                                \
            }                                                                  \
          }                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  mean = sum / ( ipFloat8_t ) size_seed;                                       \
                                                                               \
                                                                               \
  /* standard deviation                                                     */ \
                                                                               \
  saq = 0.;                                                                    \
  for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )                           \
  {                                                                            \
    off[7] = size[7] * ind[7];                                                 \
    for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )                         \
    {                                                                          \
      off[6] = size[6] * ind[6] + off[7];                                      \
      for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )                       \
      {                                                                        \
        off[5] = size[5] * ind[5] + off[6];                                    \
        for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )                     \
        {                                                                      \
          off[4] = size[4] * ind[4] + off[5];                                  \
          for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )                   \
          {                                                                    \
            off[3] = size[3] * ind[3] + off[4];                                \
            for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )                 \
            {                                                                  \
              off[2] = size[2] * ind[2] + off[3];                              \
              for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )               \
              {                                                                \
                off[1] = size[1] * ind[1] + off[2];                            \
                off[0] = off[1] + beg[0];                                      \
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )             \
                {                                                              \
                   diff = ( ipFloat8_t ) (( type * ) pic_old->data )[off[0]] - \
                            mean;                                              \
                   saq = saq + diff * diff;                                    \
                   off[0]++;                                                   \
                }                                                              \
              }                                                                \
            }                                                                  \
          }                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  std_dev = sqrt ( saq / ( ipFloat8_t ) ( size_seed - 1 ) );                   \
  std_win = std_dev * std_fact;                                                \
  printf ( "mean: %lf std_win: %lf \n", mean, std_win);\
}

#define ACPT( type, pic_old, pic_new, acpt, acpt_len, region_label, std_win )  \
{                                                                              \
  ipInt4_t     ind[_mitkIpPicNDIM];    /* loop index vector                     */ \
  ipUInt4_t    off[_mitkIpPicNDIM];    /* used to calculate offset              */ \
  ipFloat8_t   diff;               /* difference between pixel and mean     */ \
                                                                               \
  /* find pixels in seed region which are accepted                          */ \
                                                                               \
  for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )                           \
  {                                                                            \
    off[7] = size[7] * ind[7];                                                 \
    for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )                         \
    {                                                                          \
      off[6] = size[6] * ind[6] + off[7];                                      \
      for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )                       \
      {                                                                        \
        off[5] = size[5] * ind[5] + off[6];                                    \
        for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )                     \
        {                                                                      \
          off[4] = size[4] * ind[4] + off[5];                                  \
          for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )                   \
          {                                                                    \
            off[3] = size[3] * ind[3] + off[4];                                \
            for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )                 \
            {                                                                  \
              off[2] = size[2] * ind[2] + off[3];                              \
              for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )               \
              {                                                                \
                off[1] = size[1] * ind[1] + off[2];                            \
                off[0] = off[1] + beg[0];                                      \
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )             \
                {                                                              \
                   diff = fabs ( ( ipFloat8_t )                                \
                                 (( type * )pic_old->data )[off[0]] - mean );  \
                                                                               \
                   /* pixel accepted                                        */ \
                                                                               \
                   if ( diff <= std_win )                                      \
                     {                                                         \
                        acpt[acpt_len] = off[0];                               \
                        acpt_len++;                                            \
                        (( type * ) pic_new->data )[off[0]] =                  \
                           ( type ) region_label;                              \
                     }                                                         \
                   off[0]++;                                                   \
                }                                                              \
              }                                                                \
            }                                                                  \
          }                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
}

#define REGGROW( type, pic_old, pic_new, acpt, acpt_len,                       \
                 border, border_len, std_win )                                 \
{                                                                              \
  ipUInt4_t   i, j;             /* loop variables                           */ \
  ipInt4_t    k;                /* loop variable                            */ \
  ipInt4_t    rest;             /*                                          */ \
  ipInt4_t    z2;               /*                                          */ \
  ipInt4_t    z[_mitkIpPicNDIM];    /*                                          */ \
  ipBool_t    found;            /* found flag                               */ \
  ipBool_t    out;              /* offset out of image flag                 */ \
  ipFloat8_t  diff;             /* difference between pixel and mean        */ \
  ipUInt4_t   offset;           /* offset of pixel in image                 */ \
                                                                               \
  while ( acpt_len > 0 )                                                       \
  {                                                                            \
     for ( i = 0; i < acpt_len; i++ )                                          \
     {                                                                         \
       rest = ( ipInt4_t ) acpt[i];                                            \
       for ( k = pic_old->dim-1; k >= 0; k-- )                                 \
         {                                                                     \
             z[k] = rest / ( ipInt4_t )size[k];                                \
             rest = rest % ( ipInt4_t )size[k];                                \
         }                                                                     \
                                                                               \
       for ( j = 0; j < no_neigh; j++ )                                        \
       {                                                                       \
         rest = off_vekt[j];                                                   \
         out  = ipFalse;                                                       \
         for ( k = pic_old->dim-1; k >= 0; k-- )                               \
           {                                                                   \
              z2   = rest / ( ipInt4_t )size[k] + z[k];                        \
              rest = rest % ( ipInt4_t )size[k];                               \
              if ( z2 >= ( ipInt4_t ) pic_old->n[k] || z2 < 0 ) out = ipTrue;  \
           }                                                                   \
                                                                               \
         offset = ( ipUInt4_t )( ( ipInt4_t ) acpt[i] + off_vekt[j] );         \
         if ( ( !out ) &&                                                      \
              (( type * )pic_new->data )[offset] != ( type ) region_label )    \
           {                                                                   \
               diff = fabs ( ( ipFloat8_t )                                    \
                             (( type * )pic_old->data )[offset] - mean );      \
               if ( diff <= std_win )                                          \
                 {                                                             \
                    (( type * )pic_new->data )[offset] = ( type ) region_label;\
                    next[next_len] = offset;                                   \
                    next_len++;                                                \
                 }                                                             \
               else                                                            \
                 {                                                             \
                    found = ipFalse;                                           \
                    k     = 0;                                                 \
                    while ( ( !found ) && k < border_len )                     \
                      {                                                        \
                         if ( border[k] == offset ) found = ipTrue;            \
                         k++;                                                  \
                      }                                                        \
                    if ( !found )                                              \
                      {                                                        \
                         border[border_len] = offset;                          \
                         border_len++;                                         \
                      }                                                        \
                 }                                                             \
           }                                                                   \
       }                                                                       \
     }                                                                         \
     acpt_len = next_len;                                                      \
     next_len = 0;                                                             \
     help     = acpt;                                                          \
     acpt     = next;                                                          \
     next     = help;                                                          \
  }                                                                            \
  if ( kind == 0 )                                                             \
    {                                                                          \
       ipPicFree ( pic_new );                                                  \
       pic_new = ipPicClone ( pic_old );                                       \
       for ( i = 0; i < border_len; i++ )                                      \
         (( type * )pic_new->data )[border[i]] = border_label;                 \
    }                                                                          \
}
/* -------------------------------------------------------------------------- */
/*
** ipFuncRegGrow
*/
/* -------------------------------------------------------------------------- */ 

ipPicDescriptor *ipFuncRegGrow ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       dim_seed,
                                 ipUInt4_t       *beg_seed,
                                 ipUInt4_t       *end_seed,
                                 ipUInt4_t       border_label,
                                 ipUInt4_t       region_label,
                                 ipFloat8_t      std_fact,  
                                 ipUInt4_t       kind ) 
{
  ipPicDescriptor *pic_new;           /* pointer to new image                */
  ipUInt4_t        i;                     /* loop index                          */
  ipInt4_t         ind[_mitkIpPicNDIM];   /* loop index vector                   */
  ipInt4_t         beg[_mitkIpPicNDIM];   /* beginning and end of seed region    */
  ipInt4_t         end[_mitkIpPicNDIM];   /* for each dimension                  */
  ipUInt4_t        size[_mitkIpPicNDIM];  /*                                     */
  ipInt4_t         off[_mitkIpPicNDIM];   /* used to calculate offsets           */
  ipInt4_t         offset_v;              /* offset                              */
  ipInt4_t         *off_vekt;         /* offsets of all neighbour pixels     */
  ipUInt4_t        no_neigh;          /* number of neighbour pixels          */
  
  /* parameters of seed region                                               */

  ipFloat8_t       mean;              /* mean of greyvalues in seed region   */
  /*ipFloat8_t       std_dev; */          /* standard deviation                  */
  ipFloat8_t       std_win;           /* value for growing criteria          */
  /*ipFloat8_t       sum; */              /* sum of greyvalues in seed region    */
  /*ipFloat8_t       diff; */            /* difference between greyvalue, mean  */
  /*ipFloat8_t       saq; */              /* sum of deviation squares            */
  ipUInt4_t        size_seed;         /* number of pixels in seed region     */

  /* variables for region growing                                            */

  ipUInt4_t        acpt_len;          /* number of accepted pixels           */
  ipUInt4_t        next_len;          /* number of acc. pixels for next step */
  ipUInt4_t        border_len;        /* number of border pixels             */
  ipUInt4_t        *acpt;             /* offsets of all accepted pixels      */
  ipUInt4_t        *next;             /* offsets of acc. pixels for next step*/
  ipUInt4_t        *help;             /*                                     */
  ipUInt4_t        *border;           /* offsets of all border pixels        */
  
  /* check image data                                                        */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );               

  /* check parameters                                                        */

  if ( dim_seed > pic_old->dim ) return ( mitkIpFuncERROR );
  for ( i = 0; i < dim_seed; i++ )
    {
       if ( end_seed[i] <= beg_seed[i] || end_seed[i] > pic_old->n[i] ||
            beg_seed[i] < 0 )           
         {
            _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
            return ( mitkIpFuncERROR );
         }
    }

  /* allocate vectors                                                        */

  no_neigh = ( ipUInt4_t ) pow ( 3., pic_old->dim );        

  off_vekt = malloc ( no_neigh * sizeof ( ipInt4_t ) );
  if ( off_vekt == NULL ) 
    { 
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  
  acpt = malloc ( _ipPicElements ( pic_old ) * sizeof ( ipUInt4_t ) );
  if ( acpt == NULL ) 
    { 
       free ( off_vekt );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  next = malloc ( _ipPicElements ( pic_old ) * sizeof ( ipUInt4_t ) );
  if ( next == NULL )
    { 
       free ( off_vekt );
       free ( acpt );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /*
  help = malloc ( _ipPicElements ( pic_old ) * sizeof ( ipUInt4_t ) );
  if ( help == NULL ) 
    { 
       free ( off_vekt );
       free ( acpt );
       free ( next );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  */

  border = malloc ( _ipPicElements ( pic_old ) * sizeof ( ipUInt4_t ) );
  if ( border == NULL ) 
    { 
       free ( off_vekt );
       free ( acpt );
       free ( next );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  
  /* allocate pic_new and initialize it to zero                              */

  pic_new = ipPicCopyHeader ( pic_old, NULL );
  if ( pic_new == NULL ) 
    { 
       free ( off_vekt );
       free ( acpt );
       free ( next );
       free ( border );
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }
  pic_new->data = calloc ( _ipPicElements (pic_new), pic_new->bpe / 8 );
  if ( pic_new->data == NULL ) 
    { 
       free ( off_vekt );
       free ( acpt );
       free ( next );
       free ( border );
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialize variables and vectors                                        */

  size[0] = 1;
  for ( i = 1; i < pic_old->dim; i++ ) 
    size[i] = size[i-1] * pic_old->n[i-1];
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    size[i] = 0;

  for ( i = 0; i < _mitkIpPicNDIM; i++ )
    {
        beg[i] = ( i < pic_old->n[i] ) ? -1 : 0 ;
        end[i] = ( i < pic_old->n[i] ) ?  2 : 1 ;
    }
  
  acpt_len = 0;
  next_len = 0;
  border_len = 0;
   
  /* calculate offset vector                                                 */

  offset_v = 0;
  for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )
  {
    off[7] = size[7] * ind[7];
    for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )
    {
      off[6] = size[6] * ind[6] + off[7];
      for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )
      {
        off[5] = size[5] * ind[5] + off[6];
        for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )
        {
          off[4] = size[4] * ind[4] + off[5];
          for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )
          {
            off[3] = size[3] * ind[3] + off[4];
            for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )
            {
              off[2] = size[2] * ind[2] + off[3];
              for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )
              {
                off[1] = size[1] * ind[1] + off[2];
                off[0] = off[1] + beg[0]; 
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )
                {
                  off_vekt[offset_v] = off[0];
                  off[0]++;
                  offset_v++;
                }
              }
            }
          }
        }
      }
    }
  }

  /* calculate parameters of seed region                                     */

  size_seed = 1;
   
  for ( i = 0; i < dim_seed; i++ )
    {
       beg[i] = beg_seed[i];
       end[i] = end_seed[i];
       size_seed = size_seed * ( end[i] - beg[i] );
    }

  for ( i = dim_seed; i < _mitkIpPicNDIM; i++ )
    {
       beg[i] = 0;
       end[i] = 1;
    }
  
  mitkIpPicFORALL_6 ( STAT, pic_old, pic_new, mean, std_win, size_seed, beg, end );

  /* find accepted pixels in seed region                                     */
 
  mitkIpPicFORALL_5 ( ACPT, pic_old, pic_new, acpt, acpt_len, region_label, std_win );

  /* region growing                                                          */

  mitkIpPicFORALL_6 ( REGGROW, pic_old, pic_new, acpt, acpt_len, 
                  border, border_len, std_win );

  /* free memory                                                             */

  free ( off_vekt );
  free ( acpt );
  free ( next );
 /* free ( help );*/
  free ( border );
 
  /* Copy Tags */

  strncpy( pic_new->info->version, pic_old->info->version, _mitkIpPicTAGLEN );
  pic_new->info->tags_head = _ipPicCloneTags( pic_old->info->tags_head );
  pic_new->info->write_protect = pic_old->info->write_protect;
                        
  return ( pic_new );
  
}
                             
#endif                             

