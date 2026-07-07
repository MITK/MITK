/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkCESTImageNormalizationFilter_h
#define mitkCESTImageNormalizationFilter_h

#include <MitkCESTExports.h>

// MITK
#include <mitkImageToImageFilter.h>

namespace mitk
{
  /**
   \brief Normalization filter for CEST images using M0 reference images.

   This filter normalizes CEST images by dividing each voxel value by the corresponding
   M0 (normalization) reference voxel value. It only works with 4D images and assumes that the input
   mitk::Image has a property called "CEST.Offsets" (mitk::CEST_PROPERTY_NAME_OFFSETS), with offsets
   separated by spaces. The number of offsets must match the number of timesteps.

   Each timestep with a corresponding offset greater than 299 or less than -299 will be interpreted
   as a normalization (M0) image. If only one M0 image is present, normalization is done by dividing
   the voxel value by the corresponding M0 voxel value. If multiple M0 images are present,
   normalization between any two M0 images is done by dividing by a linear interpolation between them.
   The M0 images themselves are removed from the result.

   The output image will have:
   - The same 3D geometry as the input image
   - A time geometry consisting only of non-M0 timesteps
   - Double pixel type
   - Updated "CEST.Offsets" property without M0 offset values
   - Removed DICOM UIDs (series, study, SOP instance)

   \pre The input image must be a 4D image.
   \pre The input image must have a "CEST.Offsets" property with space-separated offset values.
   \pre The number of offsets must match the number of timesteps.
   \pre At least one offset must be an M0 indicator (absolute value > 299).

   \sa CustomTagParser, ExtractCESTOffset, IsNotNormalizedCESTImage, ImageToImageFilter
   */
  class MITKCEST_EXPORT CESTImageNormalizationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CESTImageNormalizationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    /*!
    \brief Standard constructor.
    */
    CESTImageNormalizationFilter();
    /*!
    \brief Standard destructor.
    */
    ~CESTImageNormalizationFilter() override;
    /*!
    \brief Generate the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    void GenerateOutputInformation() override;
    /*!
    \brief Generate the normalized output image.

    Performs the actual M0 normalization across all timesteps. M0 timesteps are identified,
    linear interpolation weights are computed between adjacent M0 images, and each non-M0
    timestep is divided by the interpolated M0 value. The time geometry is updated to exclude
    M0 timesteps.

    \throw mitk::Exception if the input image is not 4D.
    */
    void GenerateData() override;

    /** Internal templated method that normalizes across timesteps.
    \tparam TPixel The pixel type of the input image.
    \tparam VImageDimension The dimensionality of the input image (must be 4).
    \param[in] image The ITK image to normalize.
    */
    template <typename TPixel, unsigned int VImageDimension>
    void NormalizeTimeSteps(const itk::Image<TPixel, VImageDimension>* image);

    /// Offsets without M0s, space-separated, stored after normalization
    std::string m_RealOffsets;

    /// Indices of non-M0 timesteps in the original input image
    std::vector< unsigned int > m_NonM0Indices;

  };

  /**
  \brief Check whether a CEST image still contains un-normalized M0 timesteps.

  The function checks the "CEST.Offsets" property of the image and determines whether
  any of the offsets indicate an M0 normalization image (absolute value > 299).
  If such offsets exist, the image is considered not yet normalized.

  \param[in] cestImage Pointer to the CEST image to check.
  \return True if the image contains at least one M0 offset (i.e., is not normalized),
          false if all offsets are within the [-299, 299] range.
  \throw mitk::Exception if the image has no "CEST.Offsets" property or if the number of
         offsets does not match the number of timesteps.
  \sa CESTImageNormalizationFilter, ExtractCESTOffset
  */
  MITKCEST_EXPORT bool IsNotNormalizedCESTImage(const Image* cestImage);

} // END mitk namespace

#endif
