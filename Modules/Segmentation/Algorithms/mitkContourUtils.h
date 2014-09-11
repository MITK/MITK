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

#include "mitkImage.h"
#include <MitkSegmentationExports.h>
#include "mitkContour.h"
#include "mitkContourModel.h"


namespace mitk
{

/**
 * \brief Helpful methods for working with contours and images
 *
 *  Legacy support for mitk::Contour
 *  TODO remove this class when mitk::Contour is removed
 */
class MitkSegmentation_EXPORT ContourUtils : public itk::Object
{
  public:

    mitkClassMacro(ContourUtils, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    ContourModel::Pointer ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool correctionForIpSegmentation, bool constrainToInside);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

            \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, Contour* contourIn2D, bool correctionForIpSegmentation = false);


    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.
    */
    void FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue = 1 );


protected:

    ContourUtils();
    virtual ~ContourUtils();

};

}

#endif

