/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourUtilshIncludett
#define mitkContourUtilshIncludett

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
        \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and
        MITK contours)
        \param constrainToInside
      */
      ContourModel::Pointer ProjectContourTo2DSlice(Image *slice,
                                                    Contour *contourIn3D,
                                                    bool correctionForIpSegmentation,
                                                    bool constrainToInside);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param sliceGeometry
      \param contourIn2D
      \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between
      ipSegmentation and MITK contours)
    */
    ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                        Contour *contourIn2D,
                                                        bool correctionForIpSegmentation = false);

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
