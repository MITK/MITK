/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDiffusionTensor3DReconstructionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-27 17:01:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDiffusionQballGeneralizedFaImageFilter_h_
#define __itkDiffusionQballGeneralizedFaImageFilter_h_

//#include "MitkDiffusionImagingMBIExports.h"
#include "itkImageToImageFilter.h"
//#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"

namespace itk{
/** \class DiffusionQballGeneralizedFaImageFilter
 */

template< class TOdfPixelType, 
          class TGfaPixelType,
          int NrOdfDirections>
class DiffusionQballGeneralizedFaImageFilter :
  public ImageToImageFilter< Image< Vector< TOdfPixelType, NrOdfDirections >, 3 >, 
                              Image< TGfaPixelType, 3 > >
{

public:

  enum GfaComputationMethod
  {
    GFA_STANDARD,
    GFA_QUANTILES_HIGH_LOW,
    GFA_QUANTILE_HIGH,
    GFA_MAX_ODF_VALUE,
    GFA_DECONVOLUTION_COEFFS,
    GFA_MIN_MAX_NORMALIZED_STANDARD,
    GFA_NORMALIZED_ENTROPY,
    GFA_NEMATIC_ORDER_PARAMETER,
    GFA_QUANTILE_LOW,
    GFA_MIN_ODF_VALUE,
    GFA_QUANTILES_LOW_HIGH,
    GFA_STD_BY_MAX,
    GFA_PRINCIPLE_CURVATURE,
    GFA_GENERALIZED_GFA
  };

  typedef DiffusionQballGeneralizedFaImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< Image< Vector< TOdfPixelType, NrOdfDirections >, 3 >, 
    Image< TGfaPixelType, 3 > >
                          Superclass;
  
   /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Runtime information support. */
  itkTypeMacro(DiffusionQballGeneralizedFaImageFilter, 
                                                   ImageToImageFilter);
 
  typedef TOdfPixelType                 OdfComponentType;

  typedef TGfaPixelType                  DirectionPixelType;

  typedef typename Superclass::InputImageType      InputImageType;

  typedef typename Superclass::OutputImageType      OutputImageType;

  typedef typename Superclass::OutputImageRegionType
                                                   OutputImageRegionType;

  itkStaticConstMacro(NOdfDirections,int,NrOdfDirections);

  void SetOdfImage( const InputImageType *image );
  
  itkGetMacro(ComputationMethod, GfaComputationMethod);
  itkSetMacro(ComputationMethod, GfaComputationMethod);

  itkGetMacro(Param1, double);
  itkSetMacro(Param1, double);

  itkGetMacro(Param2, double);
  itkSetMacro(Param2, double);

protected:
  DiffusionQballGeneralizedFaImageFilter();
  ~DiffusionQballGeneralizedFaImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;
  
  void BeforeThreadedGenerateData();
  void ThreadedGenerateData( const 
      OutputImageRegionType &outputRegionForThread, int);

  GfaComputationMethod m_ComputationMethod;

  double m_Param1;
  double m_Param2;

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionQballGeneralizedFaImageFilter.txx"
#endif

#endif //__itkDiffusionQballGeneralizedFaImageFilter_h_

