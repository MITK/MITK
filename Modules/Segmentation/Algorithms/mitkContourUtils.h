/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourUtils_h
#define mitkContourUtils_h

#include "mitkContour.h"
#include "mitkContourModel.h"
#include "mitkImage.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
   * \brief Helpful methods for working with contours and images
   *
   *  Legacy support for mitk::Contour
   *  TODO remove this class when mitk::Contour is removed
   */
  class MITKSEGMENTATION_EXPORT ContourUtils : public itk::Object
  {
  public:
    mitkClassMacroItkParent(ContourUtils, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
        \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

        \param slice
        \param contourIn3D
      */
      ContourModel::Pointer ProjectContourTo2DSlice(Image *slice,
                                                    Contour *contourIn3D);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param sliceGeometry
      \param contourIn2D
    */
    ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                        Contour *contourIn2D);

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.
    */
    void FillContourInSlice(Contour *projectedContour, Image *sliceImage, int paintingPixelValue = 1);

  protected:
    ContourUtils();
    ~ContourUtils() override;
  };
}

#endif
