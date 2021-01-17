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
  /**
   * \brief Helpful methods for working with contours and images
   *
   *
   */
  class MITKCONTOURMODEL_EXPORT ContourModelUtils : public itk::Object
  {
  public:
    mitkClassMacroItkParent(ContourModelUtils, itk::Object);

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param slice
      \param contourIn3D
      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and
      MITK contours)
      \param constrainToInside
    */
    static ContourModel::Pointer ProjectContourTo2DSlice(const Image *slice,
                                                         const ContourModel *contourIn3D,
                                                         bool correctionForIpSegmentation,
                                                         bool constrainToInside);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param sliceGeometry
      \param contourIn2D
      \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between ipSegmentation
      and MITK contours)
    */
    static ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                               const ContourModel *contourIn2D,
                                                               bool correctionForIpSegmentation = false);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value.
    This version always uses the contour of time step 0 and fills the image.
    \pre sliceImage points to a valid instance
    \pre projectedContour points to a valid instance
    */
    static void FillContourInSlice(const ContourModel *projectedContour,
                                   Image *sliceImage,
                                   const Image* workingImage,
                                   int paintingPixelValue = 1);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value.
    This overloaded version uses the contour at the passed contourTimeStep
    to fill the passed image slice.
    \pre sliceImage points to a valid instance
    \pre projectedContour points to a valid instance
    */
    static void FillContourInSlice(const ContourModel *projectedContour,
                                   TimeStepType contourTimeStep,
                                   Image *sliceImage,
                                   const Image* workingImage,
                                   int paintingPixelValue = 1);

    /**
    \brief Fills a image (filledImage) into another image (resultImage) by incorporating the rules of LabelSet-Images
    */
    static void FillSliceInSlice(vtkSmartPointer<vtkImageData> filledImage,
                                 vtkSmartPointer<vtkImageData> resultImage,
                                 const Image* image,
                                 int paintingPixelValue);

    /**
    \brief Move the contour in time step 0 to to a new contour model at the given time step.
    */
    static ContourModel::Pointer MoveZerothContourTimeStep(const ContourModel *contour, TimeStepType timeStep);

    /**
    \brief Retrieves the active pixel value of a (labelset) image.
           If the image is basic image, the pixel value 1 (one) will be returned.
           If the image is actually a labelset image, the pixel value of the active label of the active layer will be
           returned.

    \param workingImage   The (labelset) image to retrieve the active pixel value of.
    */
    static int GetActivePixelValue(const Image* workingImage);

  protected:
    ContourModelUtils();
    ~ContourModelUtils() override;
  };
}

#endif
