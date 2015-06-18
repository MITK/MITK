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

#ifndef mitkSegmentationInterpolationAlgorithm_h_Included
#define mitkSegmentationInterpolationAlgorithm_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkImage.h"

#include <itkObjectFactory.h>

namespace mitk
{

/**
 * \brief Interface class for interpolation algorithms
 *
 * Interpolation algorithms estimate a binary image (segmentation) given
 * manual segmentations of neighboring slices. They get the following inputs:
 *
 *   - slice orientation of given and requested slices (dimension which is constant for all pixels of the meant orientation, e.g. 2 for axial).
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

    virtual Image::Pointer Interpolate(Image::ConstPointer lowerSlice, unsigned int lowerSliceIndex,
                                       Image::ConstPointer upperSlice, unsigned int upperSliceIndex,
                                       unsigned int requestedIndex,
                                       unsigned int sliceDimension,
                                       Image::Pointer resultImage,
                                       unsigned int timeStep = 0,
                                       Image::ConstPointer referenceImage = nullptr) = 0;
};

} // namespace

#endif


