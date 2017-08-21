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

#ifndef mitkShapeBasedInterpolationAlgorithm_h_Included
#define mitkShapeBasedInterpolationAlgorithm_h_Included

#include "mitkSegmentationInterpolationAlgorithm.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
   * \brief Shape-based binary image interpolation.
   *
   * This class uses legacy code from ipSegmentation to implement
   * the shape-based interpolation algorithm described in
   *
   * G.T. Herman, J. Zheng, C.A. Bucholtz: "Shape-based interpolation"
   * IEEE Computer Graphics & Applications, pp. 69-79,May 1992
   *
   *  Last contributor:
   *  $Author:$
   */
  class MITKSEGMENTATION_EXPORT ShapeBasedInterpolationAlgorithm : public SegmentationInterpolationAlgorithm
  {
  public:
    mitkClassMacro(ShapeBasedInterpolationAlgorithm, SegmentationInterpolationAlgorithm);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

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

    template <typename TPixel, unsigned int VImageDimension>
    void InterpolateIntermediateSlice(itk::Image<TPixel, VImageDimension> *result,
                                      const mitk::Image::Pointer &lowerDistanceImage,
                                      const mitk::Image::Pointer &upperDistanceImage,
                                      float ratio);
  };

} // namespace

#endif
