/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourUtils.h"
#include "mitkContourModelUtils.h"

mitk::ContourUtils::ContourUtils()
{
}

mitk::ContourUtils::~ContourUtils()
{
}

mitk::ContourModel::Pointer mitk::ContourUtils::ProjectContourTo2DSlice(Image *slice,
                                                                        Contour *contourIn3D)
{
  mitk::Contour::PointsContainerIterator it = contourIn3D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn3D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while (it != end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return mitk::ContourModelUtils::ProjectContourTo2DSlice(slice, contour);
}

mitk::ContourModel::Pointer mitk::ContourUtils::BackProjectContourFrom2DSlice(
  const BaseGeometry *sliceGeometry, Contour *contourIn2D)
{
  mitk::Contour::PointsContainerIterator it = contourIn2D->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = contourIn2D->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while (it != end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  return mitk::ContourModelUtils::BackProjectContourFrom2DSlice(sliceGeometry, contour);
}

void mitk::ContourUtils::FillContourInSlice(Contour *projectedContour, Image *sliceImage, int paintingPixelValue)
{
  mitk::Contour::PointsContainerIterator it = projectedContour->GetPoints()->Begin();
  mitk::Contour::PointsContainerIterator end = projectedContour->GetPoints()->End();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  while (it != end)
  {
    contour->AddVertex(it.Value());
    it++;
  }
  mitk::ContourModelUtils::FillContourInSlice(contour, sliceImage, sliceImage, paintingPixelValue);
}
