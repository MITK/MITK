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
* internal only. used by Scale.c
*/

/*@file
 *  this function scales the first image to the size of the second image 
 *  using bilinear interpolation 
 */

/* @brief scales the first image to the size of the second image 
 *  using bilinear interpolation 
 *
 *  @param pic_old     pointer to the image that should be scaled 
 *  @param pic_new     pointer to the scaled image ( contains the information 
 *                for the scaling in th header )
 *                !!! All Header Information necessary when calling function !!!
 *
 * @return  pointer to the scaled image 
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"   
 
ipPicDescriptor *_ipFuncScBL( ipPicDescriptor *pic_old,           
                              ipPicDescriptor *pic_new ) ;

#ifndef DOXYGEN_IGNORE

/* definition of macros                                                 */

#define SCBL4ALL( type, pic_old, pic_new, size, scale, n, shift, step, RND )  \
{                                                                        \
   ipFloat8_t       factor[_mitkIpPicNDIM];   /*                           */\
   ipFloat8_t       help1[_mitkIpPicNDIM];    /*                           */\
   size_t           help2[_mitkIpPicNDIM];    /*                           */\
   size_t           offset[_mitkIpPicNDIM];   /* offset vector             */\
   ipFloat8_t       weights[_mitkIpPicNDIM*2];/*                           */\
   size_t           i;                    /* loop index                */\
   size_t           off_new;              /* offset of scaled image    */\
   size_t           off_weights;          /* offset of orig. image     */\
   size_t           ind_o[_mitkIpPicNDIM];    /* loop index (outer loops)  */\
   size_t           ind_i[_mitkIpPicNDIM];    /* loop index (inner loops)  */\
   size_t           n_i[_mitkIpPicNDIM];      /* loop index (inner loops)  */\
   size_t           pic_elements;         /* number of elements of pic_old */ \
   type             help;                                                \
   /* initialize vectors                                               */\
                                                                         \
   for ( i = 0; i < _mitkIpPicNDIM; i++ )                                    \
     {                                                                   \
        ind_o[i] = 0;                                                    \
        ind_i[i] = 0;                                                    \
        factor[i] = 1.;                                                  \
        weights[i] = 1.;                                                 \
        weights[15-i] = 1.;                                              \
        help2[i] = 0;                                                    \
     }                                                                   \
                                                                         \
   for ( i = 0; i < pic_new->dim; i++ )                                  \
     n_i[i] = 2;                                                         \
                                                                         \
   for ( i = pic_new->dim; i < _mitkIpPicNDIM; i++ )                         \
     n_i[i] = 1;                                                         \
                                                                         \
   pic_elements = _ipPicElements(pic_old)*step;                          \
   off_new = 0;                                                          \
   switch ( pic_new->dim )                                               \
   {                                                                     \
     default: printf ( " only 8 dimensions supported \n" );              \
     case 8:                                                             \
       for ( ind_o[7] = 0; ind_o[7] < n[7]; ind_o[7]++ )                 \
       {                                                                 \
         help1[7]    = ( ipFloat8_t ) ind_o[7] * scale[7];               \
         weights[7]  = 1. - (help1 [7] - ( ipFloat8_t )                  \
                       (( size_t ) help1[7] ));                       \
         weights[15] = 1. - weights[7];                                  \
         help2[7]    = ( size_t ) help1[7] * size[7];                 \
     case 7:                                                             \
         for ( ind_o[6] = 0; ind_o[6] < n[6]; ind_o[6]++ )               \
         {                                                               \
           help1[6]    = ( ipFloat8_t ) ind_o[6] * scale[6];             \
           weights[6]  = 1. - (help1 [6] - ( ipFloat8_t )                \
                         (( size_t ) help1[6] ));                     \
           weights[14] = 1. - weights[6];                                \
           help2[6]    = ( size_t ) help1[6] * size[6];               \
     case 6:                                                             \
           for ( ind_o[5] = 0; ind_o[5] < n[5]; ind_o[5]++ )             \
           {                                                             \
             help1[5]    = ( ipFloat8_t ) ind_o[5] * scale[5];           \
             weights[5]  = 1. - (help1 [5] - ( ipFloat8_t )              \
                           (( size_t ) help1[5] ));                   \
             weights[13] = 1. - weights[5];                              \
             help2[5]    = ( size_t ) help1[5] * size[5];             \
     case 5:                                                             \
             for ( ind_o[4] = 0; ind_o[4] < n[4]; ind_o[4]++ )           \
             {                                                           \
               help1[4]    = ( ipFloat8_t ) ind_o[4] * scale[4];         \
               weights[4]  = 1. - (help1 [4] - ( ipFloat8_t )            \
                             (( size_t ) help1[4] ));                 \
               weights[12] = 1. - weights[4];                            \
               help2[4]    = ( size_t ) help1[4] * size[4];           \
     case 4:                                                             \
               for ( ind_o[3] = 0; ind_o[3] < n[3]; ind_o[3]++ )         \
               {                                                         \
                 help1[3]    = ( ipFloat8_t ) ind_o[3] * scale[3];       \
                 weights[3]  = 1. - (help1 [3] - ( ipFloat8_t )          \
                               (( size_t ) help1[3] ));               \
                 weights[11] = 1. - weights[3];                          \
                 help2[3]    = ( size_t ) help1[3] * size[3];         \
     case 3:                                                             \
                 for ( ind_o[2] = 0; ind_o[2] < n[2]; ind_o[2]++ )       \
                 {                                                       \
                   help1[2]    = ( ipFloat8_t ) ind_o[2] * scale[2];     \
                   weights[2]  = 1. - (help1 [2] - ( ipFloat8_t )        \
                                 (( size_t ) help1[2] ));             \
                   weights[10] = 1. - weights[2];                        \
                   help2[2]    = ( size_t ) help1[2] * size[2];       \
     case 2:                                                             \
                   for ( ind_o[1] = 0; ind_o[1] < n[1]; ind_o[1]++ )     \
                   {                                                     \
                     help1[1]    = ( ipFloat8_t ) ind_o[1] * scale[1];   \
                     weights[1]  = 1. - (help1 [1] - ( ipFloat8_t )      \
                                   (( size_t ) help1[1] ));           \
                     weights[9] = 1. - weights[1];                       \
                     help2[1]    = ( size_t ) help1[1] * size[1];     \
     case 1:                                                             \
                     for ( ind_o[0] = 0; ind_o[0] < n[0]; ind_o[0]++ )   \
                     {                                                   \
                       help1[0]    = ( ipFloat8_t ) ind_o[0] * scale[0]; \
                       weights[0]  = 1. - (help1 [0] - ( ipFloat8_t )    \
                                     (( size_t ) help1[0] ));         \
                       weights[8] = 1. - weights[0];                     \
                       help2[0]    = ( size_t ) help1[0] * size[0];   \
                                                                         \
                       help        = ( type ) 0;                         \
                                                                         \
                       for ( ind_i[7] = 0; ind_i[7] < n_i[7]; ind_i[7]++)\
                       {                                                 \
                         factor[7] = weights[ind_i[7]*_mitkIpPicNDIM+7];     \
                         offset[7] = ( size_t )help2[7] +             \
                                     ind_i[7] * size[7];                 \
                                                                         \
                         for ( ind_i[6] = 0; ind_i[6] < n_i[6];          \
                                             ind_i[6]++ )                \
                         {                                               \
                           factor[6] = weights[ind_i[6]*_mitkIpPicNDIM+6] *  \
                                       factor[7];                        \
                           offset[6] = ( size_t )help2[6] +           \
                                       ind_i[6] * size[6] + offset[7];   \
                                                                         \
                           for ( ind_i[5] = 0; ind_i[5] < n_i[5];        \
                                               ind_i[5]++ )              \
                           {                                             \
                             factor[5] = weights[ind_i[5]*_mitkIpPicNDIM+5] *\
                                         factor[6];                      \
                             offset[5] = ( size_t )help2[5] +         \
                                         ind_i[5] * size[5] + offset[6]; \
                                                                         \
                             for ( ind_i[4] = 0; ind_i[4] < n_i[4];      \
                                                 ind_i[4]++ )            \
                             {                                           \
                               factor[4] = weights[ind_i[4]*_mitkIpPicNDIM+4]*\
                                           factor[5];                    \
                               offset[4] = ( size_t )help2[4] +       \
                                           ind_i[4] * size[4] + offset[5];\
                                                                         \
                               for ( ind_i[3] = 0; ind_i[3] < n_i[3];    \
                                                   ind_i[3]++ )          \
                               {                                         \
                                 factor[3] = weights[ind_i[3]*_mitkIpPicNDIM+3]*\
                                             factor[4];                  \
                                 offset[3] = ( size_t )help2[3] +     \
                                             ind_i[3] * size[3] + offset[4];\
                                                                         \
                                 for ( ind_i[2] = 0; ind_i[2] < n_i[2];  \
                                       ind_i[2]++ )                      \
                                 {                                       \
                                   factor[2] = weights[ind_i[2]*_mitkIpPicNDIM + 2]*\
                                               factor[3];                \
                                   offset[2] = ( size_t )help2[2] +   \
                                               ind_i[2] * size[2] +      \
                                               offset[3];                \
                                                                         \
                                   for ( ind_i[1] = 0; ind_i[1] < n_i[1];\
                                         ind_i[1]++ )                    \
                                   {                                     \
                                     factor[1] = weights[ind_i[1]*_mitkIpPicNDIM+1] *\
                                                 factor[2];              \
                                     offset[1] = ( size_t )help2[1] + \
                                                 ind_i[1] * size[1] +    \
                                                 offset[2];              \
                                                                         \
                                     for ( ind_i[0] = 0; ind_i[0] < n_i[0];\
                                           ind_i[0]++ )                  \
                                     {                                   \
                                       offset[0] = ( size_t )(help2[0]+\
                                                   ind_i[0] * size[0] +  \
                                                   offset[1])*(( size_t )step)+shift;\
                                       if(pic_elements<=offset[0]) continue; \
                                       factor[0] = weights[ind_i[0]*_mitkIpPicNDIM] * \
                                                   factor[1];            \
                                       help = help + factor[0] *         \
                                              ((( type * ) pic_old->data )\
                                              [offset[0]]+RND);               \
                                     }                                   \
                                   }                                     \
                                 }                                       \
                               }                                         \
                             }                                           \
                           }                                             \
                         }                                               \
                       }                                                 \
                       off_new = 0;                                      \
                       for ( i = 0; i < pic_old->dim; i++ )              \
                         off_new = off_new + ind_o[i] * sizeo[i];        \
                       off_new = off_new*step+shift;                     \
                       (( type * ) pic_new->data )[off_new] = help;      \
                       off_weights = 0;                                  \
                     }                                                   \
                   }                                                     \
                 }                                                       \
               }                                                         \
             }                                                           \
           }                                                             \
         }                                                               \
       }                                                                 \
   }                                                                     \
}
#define SCBL( type, pic_old, pic_new, size, scale, n )                 \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 0, 1, 0 )
#define SCBLCOLOR( type, pic_old, pic_new, size, scale, n )            \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 0, 3, 0 )       \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 1, 3, 0 )       \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 2, 3, 0 )

#define SCBL_INT( type, pic_old, pic_new, size, scale, n )                 \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 0, 1, 0.5 )
#define SCBLCOLOR_INT( type, pic_old, pic_new, size, scale, n )            \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 0, 3, 0.5  )       \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 1, 3, 0.5  )       \
		SCBL4ALL( type, pic_old, pic_new, size, scale, n, 2, 3, 0.5  )


/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *_ipFuncScBL( ipPicDescriptor *pic_old,           
                              ipPicDescriptor *pic_new ) 
{

  ipUInt4_t       i;                  /* loopindex                      */
  ipUInt4_t        n[_mitkIpPicNDIM];     /* no. of pixels in each dimension*/
  ipFloat8_t      scale[_mitkIpPicNDIM];  /* scaling factors                */
  ipUInt4_t       size[_mitkIpPicNDIM];                                    
  ipUInt4_t       sizeo[_mitkIpPicNDIM];                                    
  char            is_color=0;


  /* check whether data are correct                                     */

  if(pic_old->bpe==24)
  {
	  is_color=1;
	  pic_old->bpe=pic_new->bpe=8;
  }

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _ipFuncError ( pic_new ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( pic_old->dim != pic_new->dim )
    {  
       _ipFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of vectors                                          */
   
  for ( i = 0; i < pic_new->dim; i++ )                                    
//	  if(pic_new->n[i]>=pic_old->n[i])
		n[i] = pic_new->n[i];//ivo  - 2;
//	  else
//		  n[i] = pic_new->n[i]-1;
                                                                          
  for ( i = pic_new->dim; i < _mitkIpPicNDIM; i++ )                           
    n[i] = 0;                                                            

  sizeo[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    sizeo[i] = sizeo[i-1] * pic_new->n[i-1];

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];

  /* calculate scaling factors                                          */

  for ( i = 0; i < pic_old->dim; i++ )
    scale[i] = ( ipFloat8_t ) ( pic_old->n[i] - 1 ) / 
               ( ipFloat8_t ) ( pic_new->n[i] - 1 );
 
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    scale[i] = 1.;
  
  /* add header information for pic_new and allocate memory             */

  pic_new->bpe  = pic_old->bpe*(is_color!=0?3:1);
  pic_new->type = pic_old->type;
  pic_new->data = malloc ( _ipPicSize ( pic_new ) );
  if ( pic_new == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new->data == NULL )
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  /* macro to scale an image ( for all data types )s)                   */

  if(pic_old->type==ipPicFloat)
  {
	if(is_color==0)
	{
		mitkIpPicFORALL_4 ( SCBL, pic_old, pic_new, size, scale, n ); 
	}
	else
	{
 		mitkIpPicFORALL_4 ( SCBLCOLOR, pic_old, pic_new, size, scale, n ); 
	}
  }
  else
  {
	if(is_color==0)
	{
		mitkIpPicFORALL_4 ( SCBL_INT, pic_old, pic_new, size, scale, n ); 
	}
	else
	{
 		mitkIpPicFORALL_4 ( SCBLCOLOR_INT, pic_old, pic_new, size, scale, n ); 
	}
  }

  return pic_new;
}
#endif
