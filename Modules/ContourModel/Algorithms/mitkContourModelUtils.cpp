/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkContourModelUtils.h>

#include <mitkContourModelToSurfaceFilter.h>
#include <mitkLabelSetImage.h>
#include <mitkSurface.h>
#include <vtkImageStencil.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>

mitk::ContourModelUtils::ContourModelUtils()
{
}

mitk::ContourModelUtils::~ContourModelUtils()
{
}

mitk::ContourModel::Pointer mitk::ContourModelUtils::ProjectContourTo2DSlice(
  const Image *slice, const ContourModel *contourIn3D, bool, bool)
{
  if (nullptr == slice || nullptr == contourIn3D)
    return nullptr;

  auto projectedContour = ContourModel::New();
  projectedContour->Initialize(*contourIn3D);

  auto sliceGeometry = slice->GetGeometry();
  auto numberOfTimesteps = static_cast<TimeStepType>(contourIn3D->GetTimeSteps());

  for (decltype(numberOfTimesteps) t = 0; t < numberOfTimesteps; ++t)
  {
    auto iter = contourIn3D->Begin(t);
    auto end = contourIn3D->End(t);

    while (iter != end)
    {
      const auto &currentPointIn3D = (*iter)->Coordinates;

      Point3D projectedPointIn2D;
      projectedPointIn2D.Fill(0.0);

      sliceGeometry->WorldToIndex(currentPointIn3D, projectedPointIn2D);

      projectedContour->AddVertex(projectedPointIn2D, t);
      ++iter;
    }
  }

  return projectedContour;
}

mitk::ContourModel::Pointer mitk::ContourModelUtils::BackProjectContourFrom2DSlice(
  const BaseGeometry *sliceGeometry, const ContourModel *contourIn2D, bool)
{
  if (nullptr == sliceGeometry || nullptr == contourIn2D)
    return nullptr;

  auto worldContour = ContourModel::New();
  worldContour->Initialize(*contourIn2D);

  auto numberOfTimesteps = static_cast<TimeStepType>(contourIn2D->GetTimeSteps());

  for (decltype(numberOfTimesteps) t = 0; t < numberOfTimesteps; ++t)
  {
    auto iter = contourIn2D->Begin(t);
    auto end = contourIn2D->End(t);

    while (iter != end)
    {
      const auto &currentPointIn2D = (*iter)->Coordinates;

      Point3D worldPointIn3D;
      worldPointIn3D.Fill(0.0);

      sliceGeometry->IndexToWorld(currentPointIn2D, worldPointIn3D);

      worldContour->AddVertex(worldPointIn3D, t);
      ++iter;
    }
  }

  return worldContour;
}

void mitk::ContourModelUtils::FillContourInSlice(
  const ContourModel *projectedContour, Image *sliceImage, const Image* workingImage, int paintingPixelValue)
{
  FillContourInSlice(projectedContour, 0, sliceImage, workingImage, paintingPixelValue);
}

void mitk::ContourModelUtils::FillContourInSlice(
  const ContourModel *projectedContour, TimeStepType contourTimeStep, Image *sliceImage, const Image* workingImage, int paintingPixelValue)
{
  if (nullptr == projectedContour)
  {
    mitkThrow() << "Cannot fill contour in slice. Passed contour is invalid";
  }

  if (nullptr == sliceImage)
  {
    mitkThrow() << "Cannot fill contour in slice. Passed slice is invalid";
  }

  auto contourModelFilter = mitk::ContourModelToSurfaceFilter::New();
  contourModelFilter->SetInput(projectedContour);
  contourModelFilter->Update();

  auto surface = mitk::Surface::New();
  surface = contourModelFilter->GetOutput();

  if (nullptr == surface->GetVtkPolyData(contourTimeStep))
  {
    MITK_WARN << "Could not create surface from contour model.";
    return;
  }

  auto surface2D = vtkSmartPointer<vtkPolyData>::New();
  surface2D->SetPoints(surface->GetVtkPolyData(contourTimeStep)->GetPoints());
  surface2D->SetLines(surface->GetVtkPolyData(contourTimeStep)->GetLines());

  auto image = vtkSmartPointer<vtkImageData>::New();
  image->DeepCopy(sliceImage->GetVtkImageData());

  const double FOREGROUND_VALUE = 255.0;
  const double BACKGROUND_VALUE = 0.0;

  vtkIdType count = image->GetNumberOfPoints();
  for (decltype(count) i = 0; i < count; ++i)
    image->GetPointData()->GetScalars()->SetTuple1(i, FOREGROUND_VALUE);

  auto polyDataToImageStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();

  // Set a minimal tolerance, so that clipped pixels will be added to contour as well.
  polyDataToImageStencil->SetTolerance(mitk::eps);
  polyDataToImageStencil->SetInputData(surface2D);
  polyDataToImageStencil->Update();

  auto imageStencil = vtkSmartPointer<vtkImageStencil>::New();

  imageStencil->SetInputData(image);
  imageStencil->SetStencilConnection(polyDataToImageStencil->GetOutputPort());
  imageStencil->ReverseStencilOff();
  imageStencil->SetBackgroundValue(BACKGROUND_VALUE);
  imageStencil->Update();

  vtkSmartPointer<vtkImageData> filledImage = imageStencil->GetOutput();
  vtkSmartPointer<vtkImageData> resultImage = sliceImage->GetVtkImageData();
  FillSliceInSlice(filledImage, resultImage, workingImage, paintingPixelValue);

  sliceImage->SetVolume(resultImage->GetScalarPointer());
}

void mitk::ContourModelUtils::FillSliceInSlice(
  vtkSmartPointer<vtkImageData> filledImage, vtkSmartPointer<vtkImageData> resultImage, const Image* image, int paintingPixelValue)
{
  auto labelImage = dynamic_cast<const LabelSetImage *>(image);
  auto numberOfPoints = filledImage->GetNumberOfPoints();

  if (nullptr == labelImage)
  {
    for (decltype(numberOfPoints) i = 0; i < numberOfPoints; ++i)
    {
      if (1 < filledImage->GetPointData()->GetScalars()->GetTuple1(i))
        resultImage->GetPointData()->GetScalars()->SetTuple1(i, paintingPixelValue);
    }
  }
  else
  {
    auto backgroundValue = labelImage->GetExteriorLabel()->GetValue();

    if (paintingPixelValue != backgroundValue)
    {
      for (decltype(numberOfPoints) i = 0; i < numberOfPoints; ++i)
      {
        if (1 < filledImage->GetPointData()->GetScalars()->GetTuple1(i))
        {
          auto existingValue = resultImage->GetPointData()->GetScalars()->GetTuple1(i);

          if (!labelImage->GetLabel(existingValue, labelImage->GetActiveLayer())->GetLocked())
            resultImage->GetPointData()->GetScalars()->SetTuple1(i, paintingPixelValue);
        }
      }
    }
    else
    {
      auto activePixelValue = labelImage->GetActiveLabel(labelImage->GetActiveLayer())->GetValue();

      for (decltype(numberOfPoints) i = 0; i < numberOfPoints; ++i)
      {
        if (1 < filledImage->GetPointData()->GetScalars()->GetTuple1(i))
        {
          if (resultImage->GetPointData()->GetScalars()->GetTuple1(i) == activePixelValue)
            resultImage->GetPointData()->GetScalars()->SetTuple1(i, paintingPixelValue);
        }
      }
    }
  }
}

mitk::ContourModel::Pointer mitk::ContourModelUtils::MoveZerothContourTimeStep(const ContourModel *contour, TimeStepType t)
{
  if (nullptr == contour)
    return nullptr;

  auto resultContour = ContourModel::New();
  resultContour->Expand(t + 1);

  std::for_each(contour->Begin(), contour->End(), [&resultContour, t](ContourElement::VertexType *vertex) {
    resultContour->AddVertex(*vertex, t);
  });

  return resultContour;
}

int mitk::ContourModelUtils::GetActivePixelValue(const Image* workingImage)
{
  auto labelSetImage = dynamic_cast<const LabelSetImage*>(workingImage);
  int activePixelValue = 1;
  if (nullptr != labelSetImage)
  {
    activePixelValue = labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer())->GetValue();
  }

  return activePixelValue;
}
