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

#include "mitkContourModelUtils.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"


#include "mitkContourModel.h"
#include "itkCastImageFilter.h"
#include "mitkImage.h"
#include "mitkSurface.h"
#include "vtkPolyData.h"
#include "mitkContourModelToSurfaceFilter.h"
#include "vtkPolyDataToImageStencil.h"
#include "vtkImageStencil.h"
#include "mitkImageVtkAccessor.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkImageLogic.h"
#include "vtkPointData.h"

#include "mitkLabelSetImage.h"

mitk::ContourModelUtils::ContourModelUtils()
{
}

mitk::ContourModelUtils::~ContourModelUtils()
{
}



mitk::ContourModel::Pointer mitk::ContourModelUtils::ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  if ( !slice || !contourIn3D ) return nullptr;

  ContourModel::Pointer projectedContour = ContourModel::New();
  projectedContour->Initialize(*contourIn3D);

  const BaseGeometry* sliceGeometry = slice->GetGeometry();

  int numberOfTimesteps = contourIn3D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
    auto iter = contourIn3D->Begin(currentTimestep);
    auto end = contourIn3D->End(currentTimestep);

    while( iter != end)
    {
      Point3D currentPointIn3D = (*iter)->Coordinates;

      Point3D projectedPointIn2D;
      projectedPointIn2D.Fill(0.0);
      sliceGeometry->WorldToIndex( currentPointIn3D, projectedPointIn2D );
      // MITK_INFO << "world point " << currentPointIn3D << " in index is " << projectedPointIn2D;

      if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) && constrainToInside )
      {
        MITK_DEBUG << "**" << currentPointIn3D << " is " << projectedPointIn2D << " --> correct it (TODO)" << std::endl;
      }

      projectedContour->AddVertex( projectedPointIn2D, currentTimestep );
      iter++;
    }
  }

  return projectedContour;
}



mitk::ContourModel::Pointer mitk::ContourModelUtils::BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, ContourModel* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ))
{
  if ( !sliceGeometry || !contourIn2D ) return nullptr;

  ContourModel::Pointer worldContour = ContourModel::New();
  worldContour->Initialize(*contourIn2D);

  int numberOfTimesteps = contourIn2D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
  auto iter = contourIn2D->Begin(currentTimestep);
  auto end = contourIn2D->End(currentTimestep);

  while( iter != end)
  {
    Point3D currentPointIn2D = (*iter)->Coordinates;

    Point3D worldPointIn3D;
    worldPointIn3D.Fill(0.0);
    sliceGeometry->IndexToWorld( currentPointIn2D, worldPointIn3D );
    //MITK_INFO << "index " << currentPointIn2D << " world " << worldPointIn3D << std::endl;

    worldContour->AddVertex( worldPointIn3D, currentTimestep );
    iter++;
  }
  }

  return worldContour;
}



void mitk::ContourModelUtils::FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, mitk::Image::Pointer workingImage, int paintingPixelValue )
{
  mitk::ContourModelUtils::FillContourInSlice(projectedContour, 0, sliceImage,workingImage, paintingPixelValue);
}


void mitk::ContourModelUtils::FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, mitk::Image::Pointer workingImage , int eraseMode)
{
      //create a surface of the input ContourModel
      mitk::Surface::Pointer surface = mitk::Surface::New();
      mitk::ContourModelToSurfaceFilter::Pointer contourModelFilter = mitk::ContourModelToSurfaceFilter::New();
      contourModelFilter->SetInput(projectedContour);
      contourModelFilter->Update();
      surface = contourModelFilter->GetOutput();

      // that's our vtkPolyData-Surface
      vtkSmartPointer<vtkPolyData> surface2D = vtkSmartPointer<vtkPolyData>::New();
      if (surface->GetVtkPolyData(timeStep) == nullptr)
      {
        MITK_WARN << "No surface has been created from contour model. Add more points to fill contour in slice.";
        return;
      }
      surface2D->SetPoints(surface->GetVtkPolyData(timeStep)->GetPoints());
      surface2D->SetLines(surface->GetVtkPolyData(timeStep)->GetLines());
      surface2D->Modified();
      //surface2D->Update();


      // prepare the binary image's voxel grid
      vtkSmartPointer<vtkImageData> whiteImage =
          vtkSmartPointer<vtkImageData>::New();
      whiteImage->DeepCopy(sliceImage->GetVtkImageData());

      // fill the image with foreground voxels:
      unsigned char inval = 255;
      unsigned char outval = 0;
      vtkIdType count = whiteImage->GetNumberOfPoints();
      for (vtkIdType i = 0; i < count; ++i)
      {
        whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
      }
      // polygonal data --> image stencil:
      vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();

      //Set a minimal tolerance, so that clipped pixels will be added to contour as well.
      pol2stenc->SetTolerance(mitk::eps);
      pol2stenc->SetInputData(surface2D);
      pol2stenc->Update();

      // cut the corresponding white image and set the background:
      vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();

      imgstenc->SetInputData(whiteImage);
      imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
      imgstenc->ReverseStencilOff();
      imgstenc->SetBackgroundValue(outval);
      imgstenc->Update();
      mitk::LabelSetImage* labelImage; // Todo: Get the working Image
      int activePixelValue = eraseMode;
      labelImage = dynamic_cast<LabelSetImage*>(workingImage.GetPointer());
      if (labelImage)
      {
        activePixelValue = labelImage->GetActiveLabel()->GetValue();
      }

      // Fill according to the Color Team
      vtkSmartPointer<vtkImageData> filledImage = imgstenc->GetOutput();
      vtkSmartPointer<vtkImageData> resultImage = sliceImage->GetVtkImageData();
      FillSliceInSlice(filledImage, resultImage, workingImage, eraseMode);
      /*
      count = filledImage->GetNumberOfPoints();
      if (activePixelValue == 0)
      {
        for (vtkIdType i = 0; i < count; ++i)
        {
          if (filledImage->GetPointData()->GetScalars()->GetTuple1(i) > 1)
          {
            resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
          }
        }
      }
      else if (eraseMode != 0) // We are not erasing...
      {
        for (vtkIdType i = 0; i < count; ++i)
        {
          if (filledImage->GetPointData()->GetScalars()->GetTuple1(i) > 1)
          {
            int targetValue = resultImage->GetPointData()->GetScalars()->GetTuple1(i);
            if (labelImage)
            {
              if (!labelImage->GetLabel(targetValue)->GetLocked())
              {
                resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
              }
            } else
            {
              resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
            }
          }
        }
      }
      else
      {
        for (vtkIdType i = 0; i < count; ++i)
        {
          if ((resultImage->GetPointData()->GetScalars()->GetTuple1(i) == activePixelValue) & (filledImage->GetPointData()->GetScalars()->GetTuple1(i) > 1))
          {
            resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
          }
        }
      }*/
      sliceImage->SetVolume(resultImage->GetScalarPointer());
}

void mitk::ContourModelUtils::FillSliceInSlice(vtkSmartPointer<vtkImageData> filledImage, vtkSmartPointer<vtkImageData> resultImage, mitk::Image::Pointer image, int eraseMode)
{
  mitk::LabelSetImage* labelImage; // Todo: Get the working Image
  int activePixelValue = eraseMode;
  labelImage = dynamic_cast<LabelSetImage*>(image.GetPointer());
  if (labelImage)
  {
    activePixelValue = labelImage->GetActiveLabel()->GetValue();
  }

  int count = filledImage->GetNumberOfPoints();
  if (activePixelValue == 0)
  {
    for (vtkIdType i = 0; i < count; ++i)
    {
      if (filledImage->GetPointData()->GetScalars()->GetTuple1(i) > 1)
      {
        resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
      }
    }
  }
  else if (eraseMode != 0) // We are not erasing...
  {
    for (vtkIdType i = 0; i < count; ++i)
    {
      if (filledImage->GetPointData()->GetScalars()->GetTuple1(i) > 1)
      {
        int targetValue = resultImage->GetPointData()->GetScalars()->GetTuple1(i);
        if (labelImage)
        {
          if (!labelImage->GetLabel(targetValue)->GetLocked())
          {
            resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
          }
        } else
        {
          resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
        }
      }
    }
  }
  else
  {
    for (vtkIdType i = 0; i < count; ++i)
    {
      if ((resultImage->GetPointData()->GetScalars()->GetTuple1(i) == activePixelValue) & (filledImage->GetPointData()->GetScalars()->GetTuple1(i) > 1))
      {
        resultImage->GetPointData()->GetScalars()->SetTuple1(i, eraseMode);
      }
    }
  }
}
