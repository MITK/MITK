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

#ifndef mitkContourModelUtilshIncludett
#define mitkContourModelUtilshIncludett

#include "mitkImage.h"
#include <MitkContourModelExports.h>
#include "mitkContourModel.h"

#include <itkImage.h>

namespace mitk
{

/**
 * \brief Helpful methods for working with contours and images
 *
 *
 */
class MitkContourModel_EXPORT ContourModelUtils : public itk::Object
{
  public:

    mitkClassMacro(ContourModelUtils, itk::Object);



    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    static ContourModel::Pointer ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool correctionForIpSegmentation, bool constrainToInside);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    static ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, ContourModel* contourIn2D, bool correctionForIpSegmentation = false);

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value at time step 0.
    */
    static void FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, int paintingPixelValue = 1 );

    /**
    \brief Fill a contour in a 2D slice with a specified pixel value at a given time step.
    */
    static void FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, int paintingPixelValue = 1 );


protected:

    ContourModelUtils();
    virtual ~ContourModelUtils();

};

}

#endif

