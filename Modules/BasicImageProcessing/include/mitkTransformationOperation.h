/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransformationOperation_h
#define mitkTransformationOperation_h

#include <mitkImage.h>
#include <MitkBasicImageProcessingExports.h>
#include <mitkImageMappingHelper.h>

namespace mitk
{

  /**
  \brief Specifies the boundary condition for image padding in wavelet transforms.
  \sa TransformationOperation::WaveletForward
  */
  enum BorderCondition
  {
    Constant,        ///< Constant (zero) boundary padding.
    Periodic,        ///< Periodic (wrap-around) boundary padding.
    ZeroFluxNeumann  ///< Zero-flux Neumann boundary padding (gradient is zero at boundary).
  };

  /**
  \brief Specifies the isotropic wavelet function type for wavelet decomposition.
  \sa TransformationOperation::WaveletForward
  */
  enum WaveletType
  {
    Held,       ///< Held isotropic wavelet.
    Vow,        ///< Vow isotropic wavelet.
    Simoncelli, ///< Simoncelli isotropic wavelet.
    Shannon     ///< Shannon isotropic wavelet.
  };

  /**
  \brief Specifies how the resampled grid is positioned relative to the original image.
  \sa TransformationOperation::ResampleImage, TransformationOperation::ResampleMask
  */
  enum GridInterpolationPositionType
  {
    SameSize,      ///< Adjust spacing so the resampled image has the same physical extent (number of voxels may change).
    OriginAligned, ///< Keep the origin aligned; the output extent may differ from the input.
    CenterAligned  ///< Center-align the resampled grid so the image center is preserved.
  };

  /**
  \brief Provides static methods for image transformations including multi-resolution decomposition,
         Laplacian of Gaussian filtering, wavelet transforms, and image resampling.

  The TransformationOperation class offers a set of static methods for spatial and frequency-domain
  transformations on mitk::Image objects. These include:
  - Multi-resolution Gaussian pyramid decomposition
  - Laplacian of Gaussian edge detection
  - Forward wavelet decomposition with selectable wavelet families
  - Image resampling with configurable interpolation and grid positioning
  - Mask resampling with automatic thresholding for non-nearest-neighbor interpolators

  \sa ArithmeticOperation, MaskCleaningOperation
  */
  class MITKBASICIMAGEPROCESSING_EXPORT TransformationOperation {
  public:
    /**
    \brief Compute a multi-resolution Gaussian pyramid decomposition of an image.

    Uses ITK's RecursiveMultiResolutionPyramidImageFilter to produce a set of
    progressively lower-resolution versions of the input image. Level 0 is the
    lowest resolution.

    \param[in] image The input image to decompose.
    \param[in] numberOfLevels The number of pyramid levels to generate.
    \param[in] outputAsDouble If true, output images have double pixel type. Default: false.
    \return A vector of images, one per pyramid level, ordered from lowest to highest resolution.
    */
    static std::vector<Image::Pointer> MultiResolution(Image::Pointer & image, unsigned int numberOfLevels, bool outputAsDouble = false);

    /**
    \brief Apply a Laplacian of Gaussian (LoG) filter to an image.

    Computes the Laplacian of the image after Gaussian smoothing with the specified sigma.
    This is useful for edge detection and blob detection at a given scale.

    \param[in] image The input image to filter.
    \param[in] sigma The standard deviation of the Gaussian kernel in physical units.
    \param[in] outputAsDouble If true, the output image has double pixel type. Default: false.
    \return A new image containing the Laplacian of Gaussian filtered result.
    */
    static Image::Pointer LaplacianOfGaussian(Image::Pointer & image, double sigma, bool outputAsDouble = false);

    /**
    \brief Compute a forward wavelet decomposition of an image.

    Performs a frequency-domain wavelet decomposition using the specified isotropic wavelet
    function. The input image is FFT-padded with the chosen boundary condition before
    transformation.

    \param[in] image The input image to decompose.
    \param[in] numberOfLevels The number of decomposition levels.
    \param[in] numberOfBands The number of high-pass sub-bands per level.
    \param[in] condition The boundary condition for FFT padding.
    \param[in] waveletType The isotropic wavelet function to use.
    \return A vector of images representing the wavelet sub-bands. For each level,
            numberOfBands images are produced, plus one low-pass image at the coarsest level.
    \sa BorderCondition, WaveletType
    */
    static std::vector<Image::Pointer> WaveletForward(Image::Pointer & image, unsigned int numberOfLevels, unsigned int numberOfBands, BorderCondition condition, WaveletType waveletType);

    /**
    \brief Resample an image to a new voxel spacing.

    Resamples the input image to the specified spacing using the given interpolation method
    and grid positioning strategy. Only spacing components greater than 0 are applied;
    components set to 0 or less retain the original spacing.

    \param[in] image The input image to resample.
    \param[in] spacing The desired output spacing vector. Components <= 0 are ignored.
    \param[in] interpolator The interpolation method to use (e.g., Linear, NearestNeighbor, BSpline3).
    \param[in] position The grid positioning strategy controlling how the output grid
               aligns with the input image.
    \param[in] returnAsDouble If true, the image is first cast to double pixel type before resampling.
    \param[in] roundOutput If true, the resampled result is rounded back to the original pixel type.
    \return A new image resampled to the specified spacing.
    \sa GridInterpolationPositionType, ResampleMask
    */
    static Image::Pointer ResampleImage(Image::Pointer &image, mitk::Vector3D spacing, mitk::ImageMappingInterpolator::Type interpolator, GridInterpolationPositionType position, bool returnAsDouble, bool roundOutput);

    /**
    \brief Resample a binary mask image to a new voxel spacing.

    Specialized resampling for binary mask images. When using nearest-neighbor interpolation,
    this delegates directly to ResampleImage(). For other interpolation methods, the mask is
    resampled as a double image and then thresholded at 0.5 to produce a binary result,
    preserving mask boundaries more accurately than direct nearest-neighbor resampling.

    \param[in] image The input mask image to resample (unsigned short expected).
    \param[in] spacing The desired output spacing vector. Components <= 0 are ignored.
    \param[in] interpolator The interpolation method to use.
    \param[in] position The grid positioning strategy.
    \return A new binary mask image resampled to the specified spacing.
    \sa ResampleImage, GridInterpolationPositionType
    */
    static Image::Pointer ResampleMask(Image::Pointer &image, mitk::Vector3D spacing, mitk::ImageMappingInterpolator::Type interpolator, GridInterpolationPositionType position);

  };


}
#endif
