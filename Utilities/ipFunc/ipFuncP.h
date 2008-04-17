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
 * ipFuncP.h
 *---------------------------------------------------------------
 * DESCRIPTION
 *   contains the function prototypes, type definitions  and 
 *   includes for the image processing functions
 *
 * AUTHOR
 *   Antje Schroeder
 *
 * UPDATES
 *
 *---------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) Heidelberg, FRG
 */


#ifndef _mitkIpFuncP_h
#define _mitkIpFuncP_h

/* include files */

#include "ipFunc.h"

/* type definitions */

typedef enum
  {
    ipFuncEroF       = 0,
    ipFuncDilaF      = 1,
    ipFuncOpenF      = 2,
    ipFuncCloseF     = 3,
    ipFuncFlagF_MAX
  } _ipFuncFlagF_t;
 
typedef struct
  { 
     ipUInt4_t   length;
     ipInt4_t    *off_vekt;
     ipFloat8_t  *mask_vekt;
  }  ipFuncMasc_t;

/* definition of macros */

#define mitkIpFuncCASE_FOR( pic, dim, ipFuncSourceIndex )\
    case dim: for( ipFuncSourceIndex[dim-1]=0; \
                   ipFuncSourceIndex[dim-1] < pic->n[dim-1]; \
                   ipFuncSourceIndex[dim-1]++ \
                 )

#define mitkIpFuncFORALL( type, pic, pic_return, \
                  ipFuncSourceIndex, ipFuncResultOffset, ipFuncResultFunction )\
  {\
  ipUInt4_t ipFuncSourceOffset=0;\
  ipUInt4_t i;\
  for( i=0; i<_mitkIpPicNDIM; i++) index[i] = 0;\
  for( i=pic->dim; i<_mitkIpPicNDIM; i++) pic->n[i] = 0;\
  for( i=pic_return->dim; i<_mitkIpPicNDIM; i++) pic_return->n[i] = 0;\
  switch( pic->dim )\
    {\
      default:\
        { \
          fprintf( stderr,\
          "sorry, only 1 to 8 dimensional data are supported.\n"); \
          pic_return = NULL;\
          break;\
        }\
      mitkIpFuncCASE_FOR( pic, 8, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 7, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 6, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 5, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 4, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 3, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 2, ipFuncSourceIndex )\
      mitkIpFuncCASE_FOR( pic, 1, ipFuncSourceIndex )\
        {\
          ipFuncResultFunction;\
          ((type *)pic_return->data)[ ipFuncResultOffset ]    \
               = ((type *)pic->data)[ ipFuncSourceOffset ] ;  \
          ipFuncSourceOffset++;\
        }\
    }\
  } 


/* function prototypes */


ipUInt4_t _ipFuncBox2d ( ipPicDescriptor *pic_old,
                         ipUInt4_t       **beg,
                         ipUInt4_t       **end );

ipUInt4_t _ipFuncBox3d ( ipPicDescriptor *pic_old,
                         ipUInt4_t       **beg,
                         ipUInt4_t       **end );

ipUInt4_t       _ipFuncError   ( ipPicDescriptor *pic );

ipPicDescriptor *_ipFuncDrawPoly( ipPicDescriptor *pic_old,        
                                 ipUInt4_t       *pol_x,   
                                 ipUInt4_t       *pol_y,    
                                 ipUInt4_t       no_pts,
                                 ipFloat8_t      *a,
                                 ipFloat8_t      *b );

ipPicDescriptor *_ipFuncOpCl   ( ipPicDescriptor *pic_old,
                                 ipPicDescriptor *pic_mask,   
                                 _ipFuncFlagF_t   kind,
                                 ipFuncFlagI_t    border );

ipPicDescriptor *_ipFuncMorph  ( ipPicDescriptor *pic_old,
                                 ipPicDescriptor *pic_mask,
                                 _ipFuncFlagF_t   kind,
                                 ipFuncFlagI_t    border );

ipFuncMasc_t *_ipFuncCompressM ( ipPicDescriptor *mask,   
                                 ipPicDescriptor *pic_old,
                                 ipFuncFlagI_t   kind,
                                 ipInt4_t        beg[_mitkIpPicNDIM],  
                                 ipInt4_t        end[_mitkIpPicNDIM] );

ipInt4_t        _ipFuncExtT    ( ipPicType_t type,   
                                 ipUInt4_t    bpe,
                                 ipFloat8_t   *min_gv,
                                 ipFloat8_t   *max_gv );


ipPicDescriptor *_ipFuncScNN   ( ipPicDescriptor *pic_old,
                                 ipPicDescriptor *pic_new );

ipPicDescriptor *_ipFuncScBL   ( ipPicDescriptor *pic_old,
                                 ipPicDescriptor *pic_new );

ipPicDescriptor *_ipFuncHitMissI( ipPicDescriptor *pic_old,
                                  ipPicDescriptor *pic_masks,
                                  ipFuncFlagI_t    border );

ipPicDescriptor *_ipFuncBorderX ( ipPicDescriptor *pic_old,
                                  ipUInt4_t       *edge,
                                  ipFloat8_t      value );

void            _ipFuncSetErrno( int error_no );

ipUInt4_t _ipGetANew( ipUInt4_t aNew[],  ipUInt4_t Index );

ipPicDescriptor *_ipFuncMalloc ( ipPicDescriptor *pic_old,
                                 ipPicDescriptor *pic_new,  
                                 ipBool_t        over_write );
#endif /* _mitkIpFuncP_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */
