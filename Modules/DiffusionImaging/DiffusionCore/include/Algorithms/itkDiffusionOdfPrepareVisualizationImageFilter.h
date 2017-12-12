/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkDiffusionOdfPrepareVisualizationImageFilter_h_
#define __itkDiffusionOdfPrepareVisualizationImageFilter_h_

#include <MitkDiffusionCoreExports.h>
#include "itkImageToImageFilter.h"
//#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include "itkDiffusionOdfGeneralizedFaImageFilter.h"

namespace itk{
/** \class DiffusionOdfPrepareVisualizationImageFilter
 */

template< class TOdfPixelType,
          int NrOdfDirections>
class MITKDIFFUSIONCORE_EXPORT DiffusionOdfPrepareVisualizationImageFilter :
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

  typedef DiffusionOdfPrepareVisualizationImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< Image< Vector< TOdfPixelType, NrOdfDirections >, 3 >,
    Image< Vector< TOdfPixelType, NrOdfDirections >, 3 > >
                          Superclass;

  typedef DiffusionOdfGeneralizedFaImageFilter<TOdfPixelType,TOdfPixelType,NrOdfDirections>
                                                  GfaFilterType;
  typedef typename GfaFilterType::OutputImageType          GfaImageType;
  typedef typename GfaFilterType::GfaComputationMethod     GfaComputationMethod;

   /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(DiffusionOdfPrepareVisualizationImageFilter,
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
  DiffusionOdfPrepareVisualizationImageFilter();
  ~DiffusionOdfPrepareVisualizationImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  void BeforeThreadedGenerateData();
  void ThreadedGenerateData( const
      OutputImageRegionType &outputRegionForThread, ThreadIdType);

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
#include "itkDiffusionOdfPrepareVisualizationImageFilter.cpp"
#endif

#endif //__itkDiffusionOdfPrepareVisualizationImageFilter_h_

