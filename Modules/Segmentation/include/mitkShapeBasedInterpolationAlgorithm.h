/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkShapeBasedInterpolationAlgorithm_h
#define mitkShapeBasedInterpolationAlgorithm_h

#include <mitkSegmentationInterpolationAlgorithm.h>
#include <MitkSegmentationExports.h>

#include <map>
#include <mutex>

namespace mitk
{
  /**
    \brief Shape-based binary image interpolation.

    This class implements the shape-based interpolation algorithm described in:

    G.T. Herman, J. Zheng, C.A. Bucholtz: "Shape-based interpolation"
    IEEE Computer Graphics & Applications, pp. 69-79, May 1992

    It computes signed distance maps from the upper and lower slices and
    linearly interpolates between them to produce the intermediate slice.
    Distance maps are cached internally for performance when interpolating
    multiple slices between the same pair.
  */
  class MITKSEGMENTATION_EXPORT ShapeBasedInterpolationAlgorithm : public SegmentationInterpolationAlgorithm
  {
  public:
    mitkClassMacro(ShapeBasedInterpolationAlgorithm, SegmentationInterpolationAlgorithm);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Interpolate a 2D slice between two known segmentation slices.
        \param lowerSlice The segmentation slice below the requested index.
        \param lowerSliceIndex The index of the lower slice.
        \param upperSlice The segmentation slice above the requested index.
        \param upperSliceIndex The index of the upper slice.
        \param requestedIndex The index of the slice to interpolate.
        \param sliceDimension The dimension along which slicing is performed.
        \param resultImage Optional pre-allocated image to write the result into.
        \param timeStep The time step to interpolate for.
        \param referenceImage A reference image providing geometry information.
        \return The interpolated 2D binary image.
      */
      Image::Pointer Interpolate(Image::ConstPointer lowerSlice,
                                 unsigned int lowerSliceIndex,
                                 Image::ConstPointer upperSlice,
                                 unsigned int upperSliceIndex,
                                 unsigned int requestedIndex,
                                 unsigned int sliceDimension,
                                 Image::Pointer resultImage,
                                 unsigned int timeStep,
                                 Image::ConstPointer referenceImage) override;

  private:
    typedef itk::Image<mitk::ScalarType, 2> DistanceFilterImageType;

    template <typename TPixel, unsigned int VImageDimension>
    void ComputeDistanceMap(const itk::Image<TPixel, VImageDimension> *, mitk::Image::Pointer &result);

    Image::Pointer ComputeDistanceMap(unsigned int sliceIndex, Image::ConstPointer slice);

    template <typename TPixel, unsigned int VImageDimension>
    void InterpolateIntermediateSlice(itk::Image<TPixel, VImageDimension> *result,
                                      const mitk::Image::Pointer &lowerDistanceImage,
                                      const mitk::Image::Pointer &upperDistanceImage,
                                      float ratio);

    std::map<unsigned int, Image::Pointer> m_DistanceImageCache;
    std::mutex m_DistanceImageCacheMutex;
  };

} // namespace

#endif
