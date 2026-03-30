/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourUtils_h
#define mitkContourUtils_h

#include <mitkContour.h>
#include <mitkContourModel.h>
#include <mitkImage.h>
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

        \param slice The 2D image slice whose geometry defines the projection plane.
        \param contourIn3D The 3D contour to project.
        \return A ContourModel expressed in 2D slice index coordinates.
      */
      ContourModel::Pointer ProjectContourTo2DSlice(Image *slice,
                                                    Contour *contourIn3D);

    /**
      \brief Projects slice index coordinates of a contour back into world coordinates.

      \param sliceGeometry The geometry of the 2D slice used for back-projection.
      \param contourIn2D The contour in 2D index coordinates.
      \return A ContourModel expressed in 3D world coordinates.
    */
    ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                        Contour *contourIn2D);

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.

      \param projectedContour The contour (in index coordinates) to fill.
      \param sliceImage The 2D image slice to fill into.
      \param paintingPixelValue The pixel value used for filling (default: 1).
    */
    void FillContourInSlice(Contour *projectedContour, Image *sliceImage, int paintingPixelValue = 1);

  protected:
    ContourUtils();
    ~ContourUtils() override;
  };
}

#endif
