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

#ifndef DIFFUSIONIMAGETRANSFORMEDCREATIONFILTER_H
#define DIFFUSIONIMAGETRANSFORMEDCREATIONFILTER_H

#include "mitkDiffusionPropertyHelper.h"
#include "mitkDiffusionImageCreationFilter.h"
#include "mitkDiffusionImageCorrectionFilter.h"

#include <MitkDiffusionCoreExports.h>


namespace mitk
{


/**
 * @brief DiffusionImageTransformedCreationFilter extends its superclass ( DiffusionImageCreationFilter ) by providing the possibility
 * to specify a list of transforms to be applied to the separate volumes before
 */
template <typename TTransformType >
class MITKDIFFUSIONCORE_EXPORT DiffusionImageTransformedCreationFilter
    : public ImageToImageFilter
{
public:
  /** class macros */
  mitkClassMacro( DiffusionImageTransformedCreationFilter,
                  ImageToImageFilter )

  itkNewMacro(Self)

  enum InterpolatorLevel
  {
    NearestNeighbor = 0,
    Linear,
    WindowedSinc
  };

  typedef std::vector< typename TTransformType::Pointer >           TransformContainerType;
  typedef std::vector< typename TTransformType::ParametersType >    TransformParametersContainerType;

  /** Set a transform container to be used, one transform per volume */
  void SetTransforms( const TransformContainerType& );

  /** Set a container of transform parameters to be used to create transforms for transforming the input */
  void SetTransformParameters( const TransformParametersContainerType& );

  /** Set interpolation type for resampling */
  void SetInterpolationLevel( InterpolatorLevel lvl)
  {
    m_InterpolationLevel = lvl;
  }

  void GenerateData() override;

  void SetResamplingReferenceImage( mitk::Image::Pointer image )
  {
    this->m_ResamplingReferenceImage = image;
  }

  void SetReferenceDiffusionImage( mitk::Image::Pointer dwimage )
  {
    this->m_DiffusionReferenceImage = dwimage;
  }

protected:
  DiffusionImageTransformedCreationFilter();
  ~DiffusionImageTransformedCreationFilter();

  DiffusionImageHeaderDescriptor GetTransformedHeaderInformation();

  TransformContainerType m_InternalTransforms;

  mitk::Image::Pointer m_ResamplingReferenceImage;

  mitk::Image::Pointer m_DiffusionReferenceImage;

  InterpolatorLevel m_InterpolationLevel;

  typedef DiffusionImageCorrectionFilter::TransformMatrixType MatrixType;
  std::vector< MatrixType > m_RotationMatrices;
};

#include "mitkDiffusionImageTransformedCreationFilter.cxx"

} // end namespace mitk

#endif // DIFFUSIONIMAGETRANSFORMEDCREATIONFILTER_H
