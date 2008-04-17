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
 *   contains the function prototypes, type definitions and 
 *   includes for the image processing functions
 */

/** @mainpage
 * @section QuickReference Quick Reference
 *
 * @link ipFunc.h List of all functions @endlink
 *
 * @link ipArithmeticAndLogical.h Arithmetic and Logical Operators @endlink
 *
 * @link ipGeometrical.h Geometrical Operators @endlink
 *
 * @link ipLocal.h Local Operators @endlink
 *
 * @link ipStatistical.h  Operators to calculate statistical values @endlink
 *
 * @link ipMorphological.h  Morphological Operators @endlink
 *
 * @link ipPointOperators.h  Point-Operators @endlink
 *
 * @link ipAdministrative.h  Administrative Functions @endlink

 * @section Introduction Introduction
 *
 * This library contains a huge set of image processing functions. It includes
 * operators of the following classes:
 * @arg Arithmetical and logical operators. This class contains as well functions
 * to add, subtract, divide or multiply two images or images and constants
 * as functions to connect two images using the logical "and" or "or" operator.
 *  @arg Operators to transform the greyvalue of each pixel indepently of its surrounding
 * pixels. In this class operators for brightness correction, contrast
 * correction or to fade out single greyvalues are included
 * @arg Local operators which use the surrounding pixel for the transformation.
 * This could be a convolution or a filtering operation to detect edges or to
 * perform a smoothing.
 *  @arg Geometrical functions which change the geometrie of the image like a
 * rotation, a reflection, a transposition or a scaling.
 *  @arg Operators to calculate statistical values. The extreme greyvalues, the
 * mean, the variance or the standard deviation of all greyvalues in an image
 * or a region of interest (rectangle, circle or an area included by a polygon)
 * are calculated.
 *  @arg Segmentation functions to find objects in an image.
 * @arg Morphological operators which change the shape of the the objects
 * shown in an image. This could be a dilation, an erosion, an opening or a
 * closing.
 * All images and convolution masks needed in theses functions have to be stored
 * in the PIC image format. Because this image format allows to handle images of
 * different datatypes and images of up to eight dimensions, the functions could
 * be used to transform all these different kind of images.
 *
 * There's also an executable included to invoke the image processing functions from the 
 * command line.
 *
 * @section Installation Installation
 * @subsection PlatformIndependent Platform Independent
 * There is a platform independent tmake-makefile called ipFunc.pro. You need to get 
 * the free tmake-utility to use it from @htmlonly <a href="ftp://ftp.troll.no/freebies/tmake/
"> 	ftp://ftp.troll.no/freebies/tmake/ </a> @endhtmlonly.
 *
 * @subsection Windows Windows
 * The library as well as the executable are precomiled using the Microsoft Developer Studio 6.0:
 * libipFuncD.lib and ipFunc.exe. The corresponding project file is ipFunc.dsw.
 * 
 * @subsection Unix Unix
 * There is a makefile, which should work on most unix platforms. There is also a special
 * makefile for Sun Solaris.
*/

#ifndef _mitkIpFunc_h
#define _mitkIpFunc_h

/* include files */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <time.h>
#include <sys/timeb.h>
#else
#include <float.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <ipPic/ipPic.h>
/* 
   Diese Datei wird in der ipFuncP.h includet
   #include <meschach/matrix2.h>
*/
#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif

/* definition of constants */

#define mitkIpFuncERROR 0
#define mitkIpFuncOK 1 
#define mitkIpFuncNOPIC_ERROR 2
#define mitkIpFuncDIM_ERROR 3
#define mitkIpFuncSIZE_ERROR 4
#define mitkIpFuncFLAG_ERROR 5
#define mitkIpFuncMALLOC_ERROR 6
#define mitkIpFuncPICNEW_ERROR 7
#define mitkIpFuncTYPE_ERROR 8
#define mitkIpFuncUNFIT_ERROR 9
#define mitkIpFuncDIMMASC_ERROR 10
#define mitkIpFuncDATA_ERROR 11
#define mitkIpFunc_ERROR 12
#define mitkIpERO 0
#define mitkIpDILA 1
#define mitkIpOPEN 0
#define mitkIpCLOSE 1
#define mitkIpREFLECT 0
#define mitkIpNOREFLECT 1
#define mitkIpOVERWRITE 1

/* type definitions */

typedef enum /* ipFuncFlagI_t */
  {
    ipFuncReflect    = 0,
    ipFuncNoReflect  = 1,
    ipFuncBorderZero = 2,
    ipFuncBorderOld  = 3,
    ipFuncKeep       = 4,
    ipFuncNoKeep     = 5,
    ipFuncScaleBl    = 6,
    ipFuncScaleNN    = 7, 
    ipFuncTotal      = 8,
    ipFuncMinMax     = 9,
    ipFuncAbove      = 10,
    ipFuncBeneath    = 11,
    ipFuncFlagI_MAX
  } ipFuncFlagI_t;

typedef struct
  {
     ipUInt4_t    x0;
     ipUInt4_t    x1;
     ipUInt4_t    y0;
     ipUInt4_t    y1;
     ipUInt4_t    z0;
     ipUInt4_t    z1;
   } ipFuncBox_t;


/* macros */
#define IPFUNCMAX( x, y ) ( ( (x) > (y) ) ? (x) : (y) )
#define IPFUNCMIN( x, y ) ( ( (x) < (y) ) ? (x) : (y) )  

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
/* extern gloabal variables */

extern int ipFuncErrno;


/* function prototypes */

ipUInt4_t ipFuncBox ( ipPicDescriptor *pic_old,
                      ipUInt4_t       **beg,
                      ipUInt4_t       **end );

ipPicDescriptor *ipFuncConvert( ipPicDescriptor *pic_old,
                                ipPicType_t     type,
                                ipUInt4_t       bpe );

void             ipFuncCopyTags(ipPicDescriptor *pic_new, ipPicDescriptor *pic_old);

int ipFuncSetTag( ipPicDescriptor *pic, char *name, int type, int size, 
                  int el_size, void *data );

ipPicDescriptor *ipFuncInv    ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncEqual  ( ipPicDescriptor *pic_old,   
                                ipFuncFlagI_t   kind,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncRefl   ( ipPicDescriptor *pic_old,
                                ipInt4_t         axis );

ipFloat8_t      ipFuncMedI    ( ipPicDescriptor *pic_old );

ipFloat8_t      ipFuncMean    ( ipPicDescriptor *pic_old );

ipFloat8_t      ipFuncVar     ( ipPicDescriptor *pic_old );

ipFloat8_t      ipFuncSDev    ( ipPicDescriptor *pic_old );

ipFloat8_t      *ipFuncGrav   ( ipPicDescriptor *pic_old );

ipFloat8_t       ipFuncMeanC  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       ipFuncVarC   ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       ipFuncSDevC  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipUInt4_t        ipFuncExtrC  ( ipPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *center,  
                                ipUInt4_t       radius );

ipPicDescriptor *ipFuncDrawPoly( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncMeanROI( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncVarROI( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncSDevROI( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipUInt4_t        ipFuncExtrROI( ipPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max, 
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       ipFuncMeanR  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       ipFuncVarR   ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       ipFuncSDevR  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipUInt4_t        ipFuncExtrR  ( ipPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipPicDescriptor *ipFuncFrame  ( ipPicDescriptor *pic_old,
                                ipUInt4_t       *edge,     
                                ipFloat8_t      value );      

ipPicDescriptor *ipFuncBorder ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *mask,     
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncBorderX ( ipPicDescriptor *pic_old,
                                 ipPicDescriptor *mask,     
                                 ipFloat8_t      value,
                                 ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncAddC   ( ipPicDescriptor *pic_old,
                                ipFloat8_t       value,
                                ipFuncFlagI_t    keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncSubC   ( ipPicDescriptor *pic_old,
                                ipFloat8_t       value,
                                ipFuncFlagI_t    keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncMultC  ( ipPicDescriptor *pic_old,
                                ipFloat8_t       value,  
                                ipFuncFlagI_t    keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncDivC   ( ipPicDescriptor *pic_old,
                                ipFloat8_t       value,  
                                ipFuncFlagI_t    keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncMultI  ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_new,
                                ipFuncFlagI_t   keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncDivI   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_new,
                                ipFuncFlagI_t   keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncAddI   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_new,  
                                ipFuncFlagI_t   keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncAddSl  ( ipPicDescriptor *pic_old,
                                ipFuncFlagI_t   keep );

ipPicDescriptor *ipFuncSubI   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_new,  
                                ipFuncFlagI_t   keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncPot    ( ipPicDescriptor *pic_1,
                                ipFloat8_t      exponent,
                                ipFuncFlagI_t   keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncSqrt   ( ipPicDescriptor *pic_1,
                                ipFuncFlagI_t   keep,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncAnd    ( ipPicDescriptor *pic_1,
                                ipPicDescriptor *pic_2,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncOr     ( ipPicDescriptor *pic_1,
                                ipPicDescriptor *pic_2,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncNot    ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncLabel  ( ipPicDescriptor *pic_old,
                                ipUInt4_t       *no_label );

ipPicDescriptor *ipFuncThresh ( ipPicDescriptor *pic_old,
                                ipFloat8_t       threshold,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncConv   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_mask,   
                                ipFuncFlagI_t    border );
 
ipPicDescriptor *ipFuncEro    ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_mask,   
                                ipFuncFlagI_t    border );

ipPicDescriptor *ipFuncDila   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_mask,   
                                ipFuncFlagI_t    border );

ipPicDescriptor *ipFuncOpen   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_mask,   
                                ipFuncFlagI_t    border );

ipPicDescriptor *ipFuncClose  ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_mask,   
                                ipFuncFlagI_t    border );

ipPicDescriptor *ipFuncHitMiss( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_masks,  
                                ipFuncFlagI_t    border );

ipPicDescriptor *ipFuncScale  ( ipPicDescriptor *pic_old,
                                ipFloat8_t      sc_fact[_mitkIpPicNDIM],
                                ipFuncFlagI_t   sc_kind );

ipPicDescriptor *ipFuncScFact ( ipPicDescriptor *pic_old,
                                ipFloat8_t      factor,               
                                ipFuncFlagI_t   sc_kind );

ipPicDescriptor *ipFuncGausF  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       len_mask,
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncCanny  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipUInt4_t       len_mask,
                                ipFloat8_t      threshold,
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncRank   ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       rank,
                                ipUInt4_t       dim_mask, 
                                ipUInt4_t       len_mask,
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncMeanF  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       len_mask,
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncShp    ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border,
                                ipUInt4_t       mask_nr );

ipPicDescriptor *ipFuncLaplace( ipPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncSobel  ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncGrad   ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncRoberts( ipPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipFuncFlagI_t   border );

ipPicDescriptor *ipFuncZeroCr ( ipPicDescriptor *pic_old );

ipPicDescriptor *ipFuncExp    ( ipPicDescriptor *pic_old,
                                ipFuncFlagI_t   kind,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncLN     ( ipPicDescriptor *pic_old );

ipPicDescriptor *ipFuncLog    ( ipPicDescriptor *pic_old );

ipPicDescriptor *ipFuncNorm   ( ipPicDescriptor *pic_old,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncNormXY ( ipPicDescriptor *pic, 
                                ipFloat8_t min_gv,
                                ipFloat8_t max_gv,
                                ipPicDescriptor *pic_return );

ipUInt4_t       ipFuncRange   ( ipPicDescriptor *pic, 
                                ipFloat8_t gv_low,
                                ipFloat8_t gv_up );

ipInt4_t        ipFuncExtr    ( ipPicDescriptor *pic, 
                                ipFloat8_t *min,
                                ipFloat8_t *max );

ipInt4_t       ipFuncInertia ( ipPicDescriptor *pic_old,   
                               ipFloat8_t      **eigen_vect,
                               ipFloat8_t      **eigen_val );

ipInt4_t       ipFuncHist   ( ipPicDescriptor *pic_old,   
                              ipFloat8_t      min_gv,  
                              ipFloat8_t      max_gv,
                              ipUInt4_t       **hist,
                              ipUInt4_t       *size_hist );

ipInt4_t       ipFuncHisto  ( ipPicDescriptor *pic_old,   
                              ipFloat8_t      *min_gv,  
                              ipFloat8_t      *max_gv,
                              ipUInt4_t       **hist,
                              ipUInt4_t       *size_hist );

ipPicDescriptor *ipFuncSelInv ( ipPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up, 
                                ipFloat8_t      gv,
                                ipPicDescriptor *pic_return );           

ipPicDescriptor *ipFuncSelect ( ipPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up, 
                                ipFloat8_t      gv,
                                ipPicDescriptor *pic_return );           

ipPicDescriptor *ipFuncSelMM  ( ipPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up,
                                ipPicDescriptor *pic_return );

ipPicDescriptor *ipFuncLevWin ( ipPicDescriptor *pic_old,   
                                ipFloat8_t      level,   
                                ipFloat8_t      window,
                                ipPicDescriptor *pic_return );           

ipPicDescriptor *ipFuncWindow ( ipPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipPicDescriptor *ipFuncWindowR( ipPicDescriptor *pic_1,        
                                ipPicDescriptor *pic_2,
                                ipUInt4_t       *begin,   
                                ipFuncFlagI_t   keep );    


ipPicDescriptor *ipFuncRegGrow ( ipPicDescriptor *pic_old,
                                 ipUInt4_t        dim_seed,
                                 ipUInt4_t       *beg_seed,
                                 ipUInt4_t       *end_seed,
                                 ipUInt4_t        border_label,
                                 ipUInt4_t        region_label,
                                 ipFloat8_t       std_fact, 
                                 ipUInt4_t        kind );

void            ipFuncPError   ( char            *string );

ipPicDescriptor *ipFuncRotate  ( ipPicDescriptor *pic,
                                 ipPicDescriptor *pic_old,
                                 int *grad, int *order );

ipPicDescriptor *ipFuncTranspose( ipPicDescriptor *pic,
                                  ipPicDescriptor *pic_old,
                                  int *permutations_vector);
      
void            ipFuncXchange   ( ipPicDescriptor **pic1,
                                  ipPicDescriptor **pic2 );

ipFloat8_t      ipFuncSkewness ( ipPicDescriptor *pic );
ipFloat8_t      ipFuncCurtosis ( ipPicDescriptor *pic );

ipFloat8_t      ipFuncSkewnessR ( ipPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

double          ipFuncCurtosisR ( ipPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

ipPicDescriptor *ipFuncFillArea ( ipPicDescriptor *pic_old,
                                  ipFuncBox_t     box,
                                  ipFloat8_t      value, 
                                  ipFuncFlagI_t   over,
                                  ipPicDescriptor *pic_return );

char            *ipFuncMakePicName ( char *pic_name, char *extension );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif /* _mitkIpFunc_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */

