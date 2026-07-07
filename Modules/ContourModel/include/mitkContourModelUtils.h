/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelUtils_h
#define mitkContourModelUtils_h

#include <mitkContourModel.h>
#include <mitkImage.h>

#include <vtkSmartPointer.h>

#include <MitkContourModelExports.h>

namespace mitk
{
  /** \brief Static utility methods for working with contour models and images.
   *
   * Provides projection, back-projection, and fill operations for contour models
   * in relation to 2D image slices. These utilities support the segmentation
   * workflow by converting between world and index coordinate contour representations
   * and by filling contour regions into images.
   *
   * \sa ContourModel, Image, LabelSetImage
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelUtils : public itk::Object
  {
  public:
    mitkClassMacroItkParent(ContourModelUtils, itk::Object);

    /** \brief Project a 3D contour onto a 2D image slice.
     *
     * Converts each vertex of the contour from world coordinates to index
     * coordinates of the given slice.
     *
     * \param[in] slice The 2D image slice providing the geometry for projection.
     * \param[in] contourIn3D The contour in world (3D) coordinates.
     * \return A new ContourModel with vertices in slice index coordinates.
     */
    static ContourModel::Pointer ProjectContourTo2DSlice(const Image *slice,
                                                         const ContourModel *contourIn3D);

    /** \brief Back-project a 2D slice contour into 3D world coordinates.
     *
     * Converts each vertex of the contour from slice index coordinates back to
     * world coordinates using the provided geometry.
     *
     * \param[in] sliceGeometry The geometry of the 2D slice.
     * \param[in] contourIn2D The contour in slice index coordinates.
     * \return A new ContourModel with vertices in world (3D) coordinates.
     */
    static ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                               const ContourModel *contourIn2D);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value.
    This version always uses the contour of time step 0 and fills the image.
    \param projectedContour Pointer to the contour that should be projected.
    \param sliceImage Pointer to the image which content should be altered by
    adding the contour with the specified paintingPixelValue.
    \param paintingPixelValue
    \pre sliceImage points to a valid instance
    \pre projectedContour points to a valid instance
    */
    static void FillContourInSlice2(const ContourModel* projectedContour,
      Image* sliceImage,
      int paintingPixelValue = 1);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value.
    This overloaded version uses the contour at the passed contourTimeStep
    to fill the passed image slice.
    \param projectedContour Pointer to the contour that should be projected.
    \param contourTimeStep
    \param sliceImage Pointer to the image which content should be altered by
    \param paintingPixelValue
    adding the contour with the specified paintingPixelValue.
    \pre sliceImage points to a valid instance
    \pre projectedContour points to a valid instance
    */
    static void FillContourInSlice2(const ContourModel* projectedContour,
      TimeStepType contourTimeStep,
      Image* sliceImage,
      int paintingPixelValue = 1);

    /** \brief Create a new contour model with the contour from time step 0 placed at the specified time step.
     * \param[in] contour The source contour model whose time step 0 data is used.
     * \param[in] timeStep The destination time step in the new contour model.
     * \return A new ContourModel containing the contour at the specified time step.
     */
    static ContourModel::Pointer MoveZerothContourTimeStep(const ContourModel *contour, TimeStepType timeStep);

  protected:
    ContourModelUtils();
    ~ContourModelUtils() override;
  };
}

#endif
