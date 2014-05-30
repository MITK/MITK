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



mitk::ContourModelUtils::ContourModelUtils()
{
}

mitk::ContourModelUtils::~ContourModelUtils()
{
}



mitk::ContourModel::Pointer mitk::ContourModelUtils::ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool itkNotUsed( correctionForIpSegmentation ), bool constrainToInside)
{
  if ( !slice || !contourIn3D ) return NULL;

  ContourModel::Pointer projectedContour = ContourModel::New();
  projectedContour->Initialize(*contourIn3D);

  const BaseGeometry* sliceGeometry = slice->GetGeometry();

  int numberOfTimesteps = contourIn3D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
    ContourModel::VertexIterator iter = contourIn3D->Begin(currentTimestep);
    ContourModel::VertexIterator end = contourIn3D->End(currentTimestep);

    while( iter != end)
    {
      Point3D currentPointIn3D = (*iter)->Coordinates;

      Point3D projectedPointIn2D;
      projectedPointIn2D.Fill(0.0);
      sliceGeometry->WorldToIndex( currentPointIn3D, projectedPointIn2D );
      // MITK_INFO << "world point " << currentPointIn3D << " in index is " << projectedPointIn2D;

      if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) && constrainToInside )
      {
        MITK_INFO << "**" << currentPointIn3D << " is " << projectedPointIn2D << " --> correct it (TODO)" << std::endl;
      }

      projectedContour->AddVertex( projectedPointIn2D, currentTimestep );
      iter++;
    }
  }

  return projectedContour;
}



mitk::ContourModel::Pointer mitk::ContourModelUtils::BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, ContourModel* contourIn2D, bool itkNotUsed( correctionForIpSegmentation ) )
{
  if ( !sliceGeometry || !contourIn2D ) return NULL;

  ContourModel::Pointer worldContour = ContourModel::New();
  worldContour->Initialize(*contourIn2D);

  int numberOfTimesteps = contourIn2D->GetTimeGeometry()->CountTimeSteps();

  for(int currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
  ContourModel::VertexIterator iter = contourIn2D->Begin(currentTimestep);
  ContourModel::VertexIterator end = contourIn2D->End(currentTimestep);

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



void mitk::ContourModelUtils::FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, int paintingPixelValue )
{
  mitk::ContourModelUtils::FillContourInSlice(projectedContour, 0, sliceImage, paintingPixelValue);
}


void mitk::ContourModelUtils::FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, int paintingPixelValue )
{
      //create a surface of the input ContourModel
      mitk::Surface::Pointer surface = mitk::Surface::New();
      mitk::ContourModelToSurfaceFilter::Pointer contourModelFilter = mitk::ContourModelToSurfaceFilter::New();
      contourModelFilter->SetInput(projectedContour);
      contourModelFilter->Update();
      surface = contourModelFilter->GetOutput();

      // that's our vtkPolyData-Surface
      vtkSmartPointer<vtkPolyData> surface2D = vtkSmartPointer<vtkPolyData>::New();
      if (surface->GetVtkPolyData(timeStep) == NULL)
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
      pol2stenc->SetTolerance(0);
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


      //Fill according to painting value
      vtkSmartPointer<vtkImageLogic> booleanOperation = vtkSmartPointer<vtkImageLogic>::New();

      booleanOperation->SetInput2Data(sliceImage->GetVtkImageData());
      booleanOperation->SetOperationToOr();
      booleanOperation->SetOutputTrueValue(1.0);

      if(paintingPixelValue == 1)
      {
        //COMBINE
        //slice or stencil
        booleanOperation->SetInputConnection(imgstenc->GetOutputPort());
        booleanOperation->SetOperationToOr();
      } else
      {
        //CUT
        //slice and not(stencil)
        vtkSmartPointer<vtkImageLogic> booleanOperationNOT = vtkSmartPointer<vtkImageLogic>::New();
        booleanOperationNOT->SetInputConnection(imgstenc->GetOutputPort());
        booleanOperationNOT->SetOperationToNot();
        booleanOperationNOT->Update();
        booleanOperation->SetInputConnection(booleanOperationNOT->GetOutputPort());
        booleanOperation->SetOperationToAnd();
      }
      booleanOperation->Update();

      //copy scalars to output image slice
      sliceImage->SetVolume(booleanOperation->GetOutput()->GetScalarPointer());
}
