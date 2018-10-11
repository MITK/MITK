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

#ifndef mitkArithmeticOperation_h
#define mitkArithmeticOperation_h

#include <mitkImage.h>
#include <MitkBasicImageProcessingExports.h>
#include <mitkImageMappingHelper.h>

namespace mitk
{

  enum BorderCondition
  {
    Constant,
    Periodic,
    ZeroFluxNeumann
  };

  enum WaveletType
  {
    Held,
    Vow,
    Simoncelli,
    Shannon
  };

  enum GridInterpolationPositionType
  {
    SameSize,
    OriginAligned,
    CenterAligned
  };

  /** \brief Executes a transformation operations on one or two images
  *
  * All parameters of the arithmetic operations must be specified during construction.
  * The actual operation is executed when calling GetResult().
  */
  class MITKBASICIMAGEPROCESSING_EXPORT TransformationOperation {
  public:
    static std::vector<Image::Pointer> MultiResolution(Image::Pointer & image, unsigned int numberOfLevels, bool outputAsDouble = false);
    static Image::Pointer LaplacianOfGaussian(Image::Pointer & image, double sigma, bool outputAsDouble = false);
    static std::vector<Image::Pointer> WaveletForward(Image::Pointer & image, unsigned int numberOfLevels, unsigned int numberOfBands, BorderCondition condition, WaveletType waveletType);

    static Image::Pointer ResampleImage(Image::Pointer &image, mitk::Vector3D spacing, mitk::ImageMappingInterpolator::Type interpolator, GridInterpolationPositionType position, bool returnAsDouble, bool roundOutput);
    static Image::Pointer ResampleMask(Image::Pointer &image, mitk::Vector3D spacing, mitk::ImageMappingInterpolator::Type interpolator, GridInterpolationPositionType position);

  };


}
#endif // mitkArithmeticOperation_h