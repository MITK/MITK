/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationInterpolationAlgorithm_h
#define mitkSegmentationInterpolationAlgorithm_h

#include <mitkCommon.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <MitkSegmentationExports.h>

#include <itkObjectFactory.h>

namespace mitk
{
  /**
   * \brief Exception thrown when a segmentation interpolation algorithm cannot
   * compute a result from its input slices.
   */
  class MITKSEGMENTATION_EXPORT SegmentationInterpolationException : public Exception
  {
  public:
    mitkExceptionClassMacro(SegmentationInterpolationException, Exception);
  };

  /**
   * \brief Interface class for interpolation algorithms
   *
   * Interpolation algorithms estimate a binary image (segmentation) given
   * manual segmentations of neighboring slices. They get the following inputs:
   *
   *   - slice orientation of given and requested slices (dimension which is constant for all pixels of the meant
   * orientation, e.g. 2 for axial).
   *   - slice indices of the neighboring slices (for upper and lower slice)
   *   - slice data of the neighboring slices (for upper and lower slice)
   *   - slice index of the requested slice (guaranteed to be between upper and lower index)
   *   - image data of the original patient image that is being segmented (optional, may not be present)
   *   - time step of the requested slice (needed to read out original image data)
   *
   *   Concrete algorithms can use e.g. itk::ImageSliceConstIteratorWithIndex to
   *   inspect the original patient image at appropriate positions - if they
   *   want to take image data into account.
   *
   *  All processing is triggered by calling Interpolate().
   *
   *  Last contributor:
   *  $Author:$
   */
  class MITKSEGMENTATION_EXPORT SegmentationInterpolationAlgorithm : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SegmentationInterpolationAlgorithm, itk::Object);

    /**
     * \brief Interpolates a binary segmentation slice between two given slices.
     *
     * \param[in] lowerSlice The segmented slice below the requested position.
     * \param[in] lowerSliceIndex The index of the lower slice.
     * \param[in] upperSlice The segmented slice above the requested position.
     * \param[in] upperSliceIndex The index of the upper slice.
     * \param[in] requestedIndex The index of the slice to interpolate (between lower and upper).
     * \param[in] sliceDimension The dimension along which slicing is performed (e.g. 2 for axial).
     * \param[in] resultImage Optional pre-allocated image to store the result.
     * \param[in] timeStep The time step for accessing the reference image data.
     * \param[in] referenceImage Optional original patient image for intensity-based interpolation.
     * \return The interpolated binary segmentation slice.
     */
    virtual Image::Pointer Interpolate(Image::ConstPointer lowerSlice,
                                       unsigned int lowerSliceIndex,
                                       Image::ConstPointer upperSlice,
                                       unsigned int upperSliceIndex,
                                       unsigned int requestedIndex,
                                       unsigned int sliceDimension,
                                       Image::Pointer resultImage,
                                       unsigned int timeStep = 0,
                                       Image::ConstPointer referenceImage = nullptr) = 0;
  };

} // namespace

#endif
