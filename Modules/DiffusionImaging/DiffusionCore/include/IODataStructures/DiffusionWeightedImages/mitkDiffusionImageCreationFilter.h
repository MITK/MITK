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

#ifndef DIFFUSIONIMAGECREATIONFILTER_H
#define DIFFUSIONIMAGECREATIONFILTER_H

#include "mitkImageToImageFilter.h"
#include "mitkDiffusionPropertyHelper.h"
#include <MitkDiffusionCoreExports.h>

namespace mitk
{

/**
 * @brief The DiffusionImageHeaderDescriptor struct bundles the necessary diffusion-weighted image header meta information.
 */
struct MITKDIFFUSIONCORE_EXPORT DiffusionImageHeaderDescriptor
{
  DiffusionImageHeaderDescriptor()
    : m_GradientDirections(nullptr)
  {
    m_BValue = -1;
  }

  float                                                           m_BValue;
  mitk::DiffusionPropertyHelper::BValueMapType                    m_BValueMapType;
  mitk::DiffusionPropertyHelper::MeasurementFrameType             m_MeasurementFrame;
  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer  m_GradientDirections;
};

/**
 * @brief The DiffusionImageCreationFilter class creates a diffusion-weighted image out of a
 * given 3D+t regular image and sufficient additional information about gradient directions and b values
 *
 * For easier use, the filter supports the usage of reference images. Here a diffusion-weighted (dw) image is expected and the meta
 * information of this image will be used for the output dw image. The diffusion information can also be specified by setting the HeaderDescriptor
 * directly (SetHeaderDescriptor).
 *
 * At least one of reference image or descriptor must be used, otherwise an exception is thrown.
 */
class MITKDIFFUSIONCORE_EXPORT DiffusionImageCreationFilter
    : public ImageToImageFilter
{
public:
  /** class macros */
  mitkClassMacro( DiffusionImageCreationFilter,
                  ImageToImageFilter )

  itkNewMacro(Self)

  typedef short                                                           DiffusionPixelType;
  typedef mitk::DiffusionPropertyHelper                                   DPH;

  typedef mitk::Image                                                     OutputType;
  typedef mitk::DiffusionPropertyHelper::ImageType                        VectorImageType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::MeasurementFrameType             MeasurementFrameType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;

  /**
   * @brief SetReferenceImage Set a diffusion image as reference, i.e. the header information will be extracted from it
   * @param reference_image A reference diffusion-weigted image - will throw exception of the input is not DW image
   */
  void SetReferenceImage( mitk::Image::Pointer reference_image );

  /**
   * @brief SetHeaderDescriptor set the information to be used with the dw image
   * @param header_descriptor
   *
   *  \sa DiffusionImageHeaderDescriptor
   */
  void SetHeaderDescriptor( DiffusionImageHeaderDescriptor header_descriptor );

  void GenerateData() override;
protected:
  DiffusionImageCreationFilter();
  ~DiffusionImageCreationFilter() override;

  GradientDirectionContainerType::Pointer InternalGetGradientDirections( );
  MeasurementFrameType InternalGetMeasurementFrame();
  float InternalGetBValue();

  VectorImageType::Pointer RemapIntoVectorImage( mitk::Image::Pointer input);

  mitk::Image::Pointer m_ReferenceImage;

  DiffusionImageHeaderDescriptor m_HeaderDescriptor;
  bool m_HeaderDescriptorSet;
};

} //end namespace mitk

#endif // DIFFUSIONIMAGECREATIONFILTER_H
