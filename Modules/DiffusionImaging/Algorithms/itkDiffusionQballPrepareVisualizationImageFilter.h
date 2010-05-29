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
#ifndef __itkDiffusionQballPrepareVisualizationImageFilter_h_
#define __itkDiffusionQballPrepareVisualizationImageFilter_h_

#include "MitkDiffusionImagingMBIExports.h"
#include "itkImageToImageFilter.h"
//#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include "itkDiffusionQballGeneralizedFaImageFilter.h"

namespace itk{
/** \class DiffusionQballPrepareVisualizationImageFilter
 */

template< class TOdfPixelType, 
          int NrOdfDirections>
class MitkDiffusionImagingMBI_EXPORT DiffusionQballPrepareVisualizationImageFilter :
  public ImageToImageFilter< Image< Vector< TOdfPixelType, NrOdfDirections >, 3 >, 
                              Image< Vector< TOdfPixelType, NrOdfDirections >, 3 > >
{

public:

  enum NormalizationMethods
  {
    PV_NONE,
    PV_MAX,
    PV_MIN_MAX,
    PV_GLOBAL_MAX,
    PV_MIN_MAX_INVERT
  };

  typedef DiffusionQballPrepareVisualizationImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< Image< Vector< TOdfPixelType, NrOdfDirections >, 3 >, 
    Image< Vector< TOdfPixelType, NrOdfDirections >, 3 > >
                          Superclass;
  
  typedef DiffusionQballGeneralizedFaImageFilter<TOdfPixelType,TOdfPixelType,NrOdfDirections>
                                                  GfaFilterType;
  typedef typename GfaFilterType::OutputImageType          GfaImageType;
  typedef typename GfaFilterType::GfaComputationMethod     GfaComputationMethod;

   /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Runtime information support. */
  itkTypeMacro(DiffusionQballPrepareVisualizationImageFilter, 
                                                   ImageToImageFilter);
 
  typedef TOdfPixelType                 OdfComponentType;

  typedef typename Superclass::InputImageType      InputImageType;

  typedef typename Superclass::OutputImageType      OutputImageType;

  typedef typename Superclass::OutputImageRegionType
                                                   OutputImageRegionType;

  itkStaticConstMacro(NOdfDirections,int,NrOdfDirections);

  itkSetMacro( Threshold, OdfComponentType );
  itkGetMacro( Threshold, OdfComponentType );

  itkSetMacro( NormalizationMethod, NormalizationMethods );
  itkGetMacro( NormalizationMethod, NormalizationMethods );

  itkSetMacro( ScaleByGfaType, GfaComputationMethod );
  itkGetMacro( ScaleByGfaType, GfaComputationMethod );

  itkSetMacro( DoScaleGfa, bool );
  itkGetMacro( DoScaleGfa, bool );

  itkSetMacro( GfaParam1, double );
  itkGetMacro( GfaParam1, double );

  itkSetMacro( GfaParam2, double );
  itkGetMacro( GfaParam2, double );

protected:
  DiffusionQballPrepareVisualizationImageFilter();
  ~DiffusionQballPrepareVisualizationImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;
  
  void BeforeThreadedGenerateData();
  void ThreadedGenerateData( const 
      OutputImageRegionType &outputRegionForThread, int);

private:

  /** Threshold on the reference image data */
  OdfComponentType                                m_Threshold;

  NormalizationMethods                            m_NormalizationMethod;

  OdfComponentType                                m_GlobalInputMaximum;

  GfaComputationMethod                            m_ScaleByGfaType;

  bool                                            m_DoScaleGfa;

  typename GfaImageType::Pointer                  m_GfaImage;

  double                                          m_GfaParam1;

  double                                          m_GfaParam2;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionQballPrepareVisualizationImageFilter.cpp"
#endif

#endif //__itkDiffusionQballPrepareVisualizationImageFilter_h_

