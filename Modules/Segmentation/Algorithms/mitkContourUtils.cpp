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

#include "mitkContourUtils.h"
#include "mitkContourModelUtils.h"


#define InstantiateAccessFunction_ItkCopyFilledContourToSlice(pixelType, dim) \
  template void mitk::ContourUtils::ItkCopyFilledContourToSlice(itk::Image<pixelType,dim>*, const mitk::Image*, int);

// explicitly instantiate the 2D version of this method
//InstantiateAccessFunctionForFixedDimension(ItkCopyFilledContourToSlice, 2);

mitk::ContourUtils::ContourUtils()
{
}


mitk::ContourUtils::~ContourUtils()
{
}


mitk::ContourModel::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  mitk::Contour::PointsContainerIterator it = contourIn3D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn3D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return mitk::ContourModelUtils::ProjectContourTo2DSlice(slice, contour, false/*not used*/, constrainToInside );
}


mitk::ContourModel::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(const Geometry3D* sliceGeometry, Contour* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  mitk::Contour::PointsContainerIterator it = contourIn2D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn2D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return mitk::ContourModelUtils::BackProjectContourFrom2DSlice(sliceGeometry, contour, false/*not used*/);
}



void mitk::ContourUtils::FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  mitk::Contour::PointsContainerIterator it = projectedContour->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = projectedContour->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while(it!=end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  mitk::ContourModelUtils::FillContourInSlice(contour, sliceImage, paintingPixelValue);
}

