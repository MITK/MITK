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
    mitkClassMacroItkParent(ContourModelUtils, itk::Object)

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and
      MITK contours)
    */
    static ContourModel::Pointer ProjectContourTo2DSlice(Image *slice,
                                                         ContourModel *contourIn3D,
                                                         bool correctionForIpSegmentation,
                                                         bool constrainToInside);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between ipSegmentation
      and MITK contours)
    */
    static ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                               ContourModel *contourIn2D,
                                                               bool correctionForIpSegmentation = false);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value at time step 0.
    */
    static void FillContourInSlice(ContourModel *projectedContour,
                                   Image *sliceImage,
                                   mitk::Image::Pointer workingImage,
                                   int paintingPixelValue = 1);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value at a given time step.
    */
    static void FillContourInSlice(ContourModel *projectedContour,
                                   unsigned int timeStep,
                                   Image *sliceImage,
                                   mitk::Image::Pointer workingImage,
                                   int paintingPixelValue = 1);

    /**
    \brief Fills a image (filledImage) into another image (resultImage) by incorporating the rules of LabelSet-Images
    */
    static void FillSliceInSlice(vtkSmartPointer<vtkImageData> filledImage,
                                 vtkSmartPointer<vtkImageData> resultImage,
                                 mitk::Image::Pointer image,
                                 int paintingPixelValue);

    /**
    \brief Move the contour in time step 0 to to a new contour model at the given time step.
    */
    static ContourModel::Pointer MoveZerothContourTimeStep(const ContourModel *contour, unsigned int timeStep);

  protected:
    ContourModelUtils();
    ~ContourModelUtils() override;
  };
}

#endif
