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
 * @link mitkIpFunc.h List of all functions @endlink
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
#include <mitkIpPic.h>
/* 
   Diese Datei wird in der mitkIpFuncP.h includet
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

typedef enum /* mitkIpFuncFlagI_t */
  {
    mitkIpFuncReflect    = 0,
    mitkIpFuncNoReflect  = 1,
    mitkIpFuncBorderZero = 2,
    mitkIpFuncBorderOld  = 3,
    mitkIpFuncKeep       = 4,
    mitkIpFuncNoKeep     = 5,
    mitkIpFuncScaleBl    = 6,
    mitkIpFuncScaleNN    = 7, 
    mitkIpFuncTotal      = 8,
    mitkIpFuncMinMax     = 9,
    mitkIpFuncAbove      = 10,
    mitkIpFuncBeneath    = 11,
    mitkIpFuncFlagI_MAX
  } mitkIpFuncFlagI_t;

typedef struct
  {
     ipUInt4_t    x0;
     ipUInt4_t    x1;
     ipUInt4_t    y0;
     ipUInt4_t    y1;
     ipUInt4_t    z0;
     ipUInt4_t    z1;
   } mitkIpFuncBox_t;


/* macros */
#define IPFUNCMAX( x, y ) ( ( (x) > (y) ) ? (x) : (y) )
#define IPFUNCMIN( x, y ) ( ( (x) < (y) ) ? (x) : (y) )  

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
/* extern gloabal variables */

extern int mitkIpFuncErrno;


/* function prototypes */

ipUInt4_t mitkIpFuncBox ( mitkIpPicDescriptor *pic_old,
                      ipUInt4_t       **beg,
                      ipUInt4_t       **end );

mitkIpPicDescriptor *mitkIpFuncConvert( mitkIpPicDescriptor *pic_old,
                                mitkIpPicType_t     type,
                                ipUInt4_t       bpe );

void             mitkIpFuncCopyTags(mitkIpPicDescriptor *pic_new, mitkIpPicDescriptor *pic_old);

int mitkIpFuncSetTag( mitkIpPicDescriptor *pic, char *name, int type, int size, 
                  int el_size, void *data );

mitkIpPicDescriptor *mitkIpFuncInv    ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncEqual  ( mitkIpPicDescriptor *pic_old,   
                                mitkIpFuncFlagI_t   kind,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncRefl   ( mitkIpPicDescriptor *pic_old,
                                ipInt4_t         axis );

ipFloat8_t      mitkIpFuncMedI    ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      mitkIpFuncMean    ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      mitkIpFuncVar     ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      mitkIpFuncSDev    ( mitkIpPicDescriptor *pic_old );

ipFloat8_t      *mitkIpFuncGrav   ( mitkIpPicDescriptor *pic_old );

ipFloat8_t       mitkIpFuncMeanC  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       mitkIpFuncVarC   ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipFloat8_t       mitkIpFuncSDevC  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       radius  );

ipUInt4_t        mitkIpFuncExtrC  ( mitkIpPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *center,  
                                ipUInt4_t       radius );

mitkIpPicDescriptor *mitkIpFuncDrawPoly( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncMeanROI( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncVarROI( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncSDevROI( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipUInt4_t        mitkIpFuncExtrROI( mitkIpPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max, 
                                ipUInt4_t       *pol_x,   
                                ipUInt4_t       *pol_y,    
                                ipUInt4_t       no_pts );

ipFloat8_t       mitkIpFuncMeanR  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       mitkIpFuncVarR   ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipFloat8_t       mitkIpFuncSDevR  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

ipUInt4_t        mitkIpFuncExtrR  ( mitkIpPicDescriptor *pic_old,        
                                ipFloat8_t      *min,
                                ipFloat8_t      *max,  
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

mitkIpPicDescriptor *mitkIpFuncFrame  ( mitkIpPicDescriptor *pic_old,
                                ipUInt4_t       *edge,     
                                ipFloat8_t      value );      

mitkIpPicDescriptor *mitkIpFuncBorder ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *mask,     
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncBorderX ( mitkIpPicDescriptor *pic_old,
                                 mitkIpPicDescriptor *mask,     
                                 ipFloat8_t      value,
                                 mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncAddC   ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t       value,
                                mitkIpFuncFlagI_t    keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncSubC   ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t       value,
                                mitkIpFuncFlagI_t    keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncMultC  ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t       value,  
                                mitkIpFuncFlagI_t    keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncDivC   ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t       value,  
                                mitkIpFuncFlagI_t    keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncMultI  ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_new,
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncDivI   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_new,
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncAddI   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_new,  
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncAddSl  ( mitkIpPicDescriptor *pic_old,
                                mitkIpFuncFlagI_t   keep );

mitkIpPicDescriptor *mitkIpFuncSubI   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_new,  
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncPot    ( mitkIpPicDescriptor *pic_1,
                                ipFloat8_t      exponent,
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncSqrt   ( mitkIpPicDescriptor *pic_1,
                                mitkIpFuncFlagI_t   keep,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncAnd    ( mitkIpPicDescriptor *pic_1,
                                mitkIpPicDescriptor *pic_2,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncOr     ( mitkIpPicDescriptor *pic_1,
                                mitkIpPicDescriptor *pic_2,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncNot    ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncLabel  ( mitkIpPicDescriptor *pic_old,
                                ipUInt4_t       *no_label );

mitkIpPicDescriptor *mitkIpFuncThresh ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t       threshold,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncConv   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_mask,   
                                mitkIpFuncFlagI_t    border );
 
mitkIpPicDescriptor *mitkIpFuncEro    ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_mask,   
                                mitkIpFuncFlagI_t    border );

mitkIpPicDescriptor *mitkIpFuncDila   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_mask,   
                                mitkIpFuncFlagI_t    border );

mitkIpPicDescriptor *mitkIpFuncOpen   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_mask,   
                                mitkIpFuncFlagI_t    border );

mitkIpPicDescriptor *mitkIpFuncClose  ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_mask,   
                                mitkIpFuncFlagI_t    border );

mitkIpPicDescriptor *mitkIpFuncHitMiss( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_masks,  
                                mitkIpFuncFlagI_t    border );

mitkIpPicDescriptor *mitkIpFuncScale  ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t      sc_fact[_mitkIpPicNDIM],
                                mitkIpFuncFlagI_t   sc_kind );

mitkIpPicDescriptor *mitkIpFuncScFact ( mitkIpPicDescriptor *pic_old,
                                ipFloat8_t      factor,               
                                mitkIpFuncFlagI_t   sc_kind );

mitkIpPicDescriptor *mitkIpFuncGausF  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       len_mask,
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncCanny  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                ipUInt4_t       len_mask,
                                ipFloat8_t      threshold,
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncRank   ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       rank,
                                ipUInt4_t       dim_mask, 
                                ipUInt4_t       len_mask,
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncMeanF  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       len_mask,
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncShp    ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border,
                                ipUInt4_t       mask_nr );

mitkIpPicDescriptor *mitkIpFuncLaplace( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncSobel  ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncGrad   ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncRoberts( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       dim_mask, 
                                mitkIpFuncFlagI_t   border );

mitkIpPicDescriptor *mitkIpFuncZeroCr ( mitkIpPicDescriptor *pic_old );

mitkIpPicDescriptor *mitkIpFuncExp    ( mitkIpPicDescriptor *pic_old,
                                mitkIpFuncFlagI_t   kind,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncLN     ( mitkIpPicDescriptor *pic_old );

mitkIpPicDescriptor *mitkIpFuncLog    ( mitkIpPicDescriptor *pic_old );

mitkIpPicDescriptor *mitkIpFuncNorm   ( mitkIpPicDescriptor *pic_old,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncNormXY ( mitkIpPicDescriptor *pic, 
                                ipFloat8_t min_gv,
                                ipFloat8_t max_gv,
                                mitkIpPicDescriptor *pic_return );

ipUInt4_t       mitkIpFuncRange   ( mitkIpPicDescriptor *pic, 
                                ipFloat8_t gv_low,
                                ipFloat8_t gv_up );

ipInt4_t        mitkIpFuncExtr    ( mitkIpPicDescriptor *pic, 
                                ipFloat8_t *min,
                                ipFloat8_t *max );

ipInt4_t       mitkIpFuncInertia ( mitkIpPicDescriptor *pic_old,   
                               ipFloat8_t      **eigen_vect,
                               ipFloat8_t      **eigen_val );

ipInt4_t       mitkIpFuncHist   ( mitkIpPicDescriptor *pic_old,   
                              ipFloat8_t      min_gv,  
                              ipFloat8_t      max_gv,
                              ipUInt4_t       **hist,
                              ipUInt4_t       *size_hist );

ipInt4_t       mitkIpFuncHisto  ( mitkIpPicDescriptor *pic_old,   
                              ipFloat8_t      *min_gv,  
                              ipFloat8_t      *max_gv,
                              ipUInt4_t       **hist,
                              ipUInt4_t       *size_hist );

mitkIpPicDescriptor *mitkIpFuncSelInv ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up, 
                                ipFloat8_t      gv,
                                mitkIpPicDescriptor *pic_return );           

mitkIpPicDescriptor *mitkIpFuncSelect ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up, 
                                ipFloat8_t      gv,
                                mitkIpPicDescriptor *pic_return );           

mitkIpPicDescriptor *mitkIpFuncSelMM  ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      gv_low,  
                                ipFloat8_t      gv_up,
                                mitkIpPicDescriptor *pic_return );

mitkIpPicDescriptor *mitkIpFuncLevWin ( mitkIpPicDescriptor *pic_old,   
                                ipFloat8_t      level,   
                                ipFloat8_t      window,
                                mitkIpPicDescriptor *pic_return );           

mitkIpPicDescriptor *mitkIpFuncWindow ( mitkIpPicDescriptor *pic_old,        
                                ipUInt4_t       *begin,   
                                ipUInt4_t       *length );

mitkIpPicDescriptor *mitkIpFuncWindowR( mitkIpPicDescriptor *pic_1,        
                                mitkIpPicDescriptor *pic_2,
                                ipUInt4_t       *begin,   
                                mitkIpFuncFlagI_t   keep );    


mitkIpPicDescriptor *mitkIpFuncRegGrow ( mitkIpPicDescriptor *pic_old,
                                 ipUInt4_t        dim_seed,
                                 ipUInt4_t       *beg_seed,
                                 ipUInt4_t       *end_seed,
                                 ipUInt4_t        border_label,
                                 ipUInt4_t        region_label,
                                 ipFloat8_t       std_fact, 
                                 ipUInt4_t        kind );

void            mitkIpFuncPError   ( char            *string );

mitkIpPicDescriptor *mitkIpFuncRotate  ( mitkIpPicDescriptor *pic,
                                 mitkIpPicDescriptor *pic_old,
                                 int *grad, int *order );

mitkIpPicDescriptor *mitkIpFuncTranspose( mitkIpPicDescriptor *pic,
                                  mitkIpPicDescriptor *pic_old,
                                  int *permutations_vector);
      
void            mitkIpFuncXchange   ( mitkIpPicDescriptor **pic1,
                                  mitkIpPicDescriptor **pic2 );

ipFloat8_t      mitkIpFuncSkewness ( mitkIpPicDescriptor *pic );
ipFloat8_t      mitkIpFuncCurtosis ( mitkIpPicDescriptor *pic );

ipFloat8_t      mitkIpFuncSkewnessR ( mitkIpPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

double          mitkIpFuncCurtosisR ( mitkIpPicDescriptor *pic_old,
                                  ipUInt4_t       *begin, 
                                  ipUInt4_t       *length );

mitkIpPicDescriptor *mitkIpFuncFillArea ( mitkIpPicDescriptor *pic_old,
                                  mitkIpFuncBox_t     box,
                                  ipFloat8_t      value, 
                                  mitkIpFuncFlagI_t   over,
                                  mitkIpPicDescriptor *pic_return );

char            *mitkIpFuncMakePicName ( char *pic_name, char *extension );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif /* _mitkIpFunc_h */ 
/* DON'T ADD ANYTHING AFTER THIS #endif */

