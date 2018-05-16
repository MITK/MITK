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

#ifndef __itkBallAndSticksImageFilter_h_
#define __itkBallAndSticksImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionPropertyHelper.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <mitkOdfImage.h>
#include <itkDiffusionTensor3D.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <cmath>
#include <mitkTensorImage.h>

#define NUM_TENSORS 2

namespace itk{
/** \class BallAndSticksImageFilter
 */

template< class TInPixelType, class TOutPixelType >
class BallAndSticksImageFilter :
    public ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >
{

public:

  typedef BallAndSticksImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >  Superclass;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType GradientContainerType;
  typedef itk::Image< unsigned char, 3>     ItkUcharImageType;
  typedef itk::Image< float, 4 >            PeakImageType;

  typedef itk::DiffusionTensor3DReconstructionImageFilter< TInPixelType, TInPixelType, float > TensorRecFilterType;
  typedef mitk::TensorImage::PixelType TensorType;
  typedef mitk::TensorImage::ItkTensorImageType TensorImageType;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(BallAndSticksImageFilter, ImageToImageFilter)

  typedef typename Superclass::InputImageType InputImageType;
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  itkSetMacro( MaskImage, ItkUcharImageType::Pointer )
  itkSetMacro( B_value, double )
  itkSetMacro( GradientDirections, GradientContainerType::Pointer )
  itkGetMacro( PeakImage, PeakImageType::Pointer )
  itkGetMacro( OutDwi, typename InputImageType::Pointer )

  protected:
    BallAndSticksImageFilter();
  ~BallAndSticksImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const override;

  void BeforeThreadedGenerateData() override;
  void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType) override;


  double                            m_B_value;
  std::vector<double>               m_B_values;
  std::vector<int>                  m_WeightedIndices;
  std::vector<int>                  m_UnWeightedIndices;
  GradientContainerType::Pointer    m_GradientDirections;

  ItkUcharImageType::Pointer        m_MaskImage;
  PeakImageType::Pointer            m_PeakImage;
  TensorImageType::Pointer          m_TensorImage;
  typename InputImageType::Pointer           m_OutDwi;

  vnl_vector<double> FitSingleVoxel( const typename InputImageType::PixelType &input, const typename InputImageType::IndexType &idx);

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBallAndSticksImageFilter.txx"
#endif

#endif //__itkBallAndSticksImageFilter_h_

