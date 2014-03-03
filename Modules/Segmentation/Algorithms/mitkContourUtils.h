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

#ifndef mitkContourUtilshIncludett
#define mitkContourUtilshIncludett

#include "SegmentationExports.h"
#include "mitkContourModel.h"
#include "mitkImage.h"
#include "mitkLegacyAdaptors.h"

#include <itkImage.h>

namespace mitk
{

/**
 * \brief Helpful methods for working with contours and images
 */
class Segmentation_EXPORT ContourUtils : public itk::Object
{
  public:

    mitkClassMacro(ContourUtils, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.
    */
    static void ProjectContourTo2DSlice(const Geometry3D* sliceGeometry, ContourModel* contourIn3D, ContourModel* contourIn2D);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.
    */
    static void BackProjectContourFrom2DSlice(const Geometry3D* sliceGeometry, ContourModel* contourIn2D, ContourModel* contourIn3D);

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.
    */
    static void FillContourInSlice( ContourModel* projectedContour, Image* slice, int paintingPixelValue );

  protected:

    ContourUtils();
    virtual ~ContourUtils();

    /**
      \brief Paint a filled contour (e.g. of an ipSegmentation pixel type) into a mitk::Image (or arbitraty pixel type).
      Will not copy the whole filledContourSlice, but only set those pixels in originalSlice that can be overwritten acording to
      the label "locked" property to overwritevalue, where the corresponding pixel in filledContourSlice is non-zero.
    */
    template<typename TPixel, unsigned int VImageDimension>
    static void ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const mitk::Image* filledContourSlice, int pixelvalue );
};

}

#endif

