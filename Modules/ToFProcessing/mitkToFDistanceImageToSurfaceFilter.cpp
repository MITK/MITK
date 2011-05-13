/*=========================================================================
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkInstantiateAccessFunctions.h>
#include <mitkSurface.h>
#include <mitkPinholeCameraModel.h>
#include <mitkToFProcessingCommon.h>

#include <itkImage.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>
#include <vtkSmartPointer.h>

#include <math.h>

mitk::ToFDistanceImageToSurfaceFilter::ToFDistanceImageToSurfaceFilter(): m_CameraModel(),
m_TextureImageWidth(0), m_TextureImageHeight(0), m_IplScalarImage(NULL)
{
  m_CameraModel = mitk::PinholeCameraModel::New();
}

mitk::ToFDistanceImageToSurfaceFilter::~ToFDistanceImageToSurfaceFilter()
{
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput( Image* distanceImage, mitk::PinholeCameraModel::Pointer CameraModel )
{
  this->SetCameraModel(CameraModel);
  this->SetInput(0,distanceImage);
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput( unsigned int idx,  Image* distanceImage, mitk::PinholeCameraModel::Pointer CameraModel )
{
  this->SetCameraModel(CameraModel);
  this->SetInput(idx,distanceImage);
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput(  mitk::Image* distanceImage )
{
  this->SetInput(0,distanceImage);
}

//TODO: braucht man diese Methode?
void mitk::ToFDistanceImageToSurfaceFilter::SetInput( unsigned int idx,  mitk::Image* distanceImage )
{
  if ((distanceImage == NULL) && (idx == this->GetNumberOfInputs() - 1)) // if the last input is set to NULL, reduce the number of inputs by one
    this->SetNumberOfInputs(this->GetNumberOfInputs() - 1);
  else
    this->ProcessObject::SetNthInput(idx, distanceImage);   // Process object is not const-correct so the const_cast is required here

  this->CreateOutputsForAllInputs();
}

mitk::Image* mitk::ToFDistanceImageToSurfaceFilter::GetInput()
{
  return this->GetInput(0);
}

mitk::Image* mitk::ToFDistanceImageToSurfaceFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL; //TODO: geeignete exception werfen
  return static_cast< mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ToFDistanceImageToSurfaceFilter::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();
  assert(output);
  mitk::Image::Pointer input = this->GetInput();
  assert(input);
  // mesh points
  unsigned int xDimension = input->GetDimension(0);
  unsigned int yDimension = input->GetDimension(1);
  unsigned int size = xDimension*yDimension; //size of the image-array
  std::vector<bool> isPointValid;
  isPointValid.resize(size);
  int pointCount = 0;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  vtkSmartPointer<vtkFloatArray> scalarArray = vtkSmartPointer<vtkFloatArray>::New();
  vtkSmartPointer<vtkFloatArray> textureCoords = vtkSmartPointer<vtkFloatArray>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  textureCoords->SetNumberOfComponents(2);

  float textureScaleCorrection1 = 0.0;
  float textureScaleCorrection2 = 0.0;
  if (this->m_TextureImageHeight > 0.0 && this->m_TextureImageWidth > 0.0)
  {
    textureScaleCorrection1 = float(this->m_TextureImageHeight) / float(this->m_TextureImageWidth);
    textureScaleCorrection2 = ((float(this->m_TextureImageWidth) - float(this->m_TextureImageHeight))/2) / float(this->m_TextureImageWidth);
  }

  float* scalarFloatData = NULL;

  if (this->m_IplScalarImage)
  {
    scalarFloatData = (float*)this->m_IplScalarImage->imageData;
  }

  float* inputFloatData = (float*)(input->GetSliceData(0, 0, 0)->GetData());

  //calculate world coordinates
  for (int j=0; j<yDimension; j++)
  {
    for (int i=0; i<xDimension; i++)
    {
      // distance value
      mitk::Index3D pixel;
      pixel[0] = i;
      pixel[1] = j;
      pixel[2] = 0;

      unsigned int pixelID = pixel[0]+pixel[1]*yDimension;

      ToFScalarType distance = (double)inputFloatData[pixelID];
      //ToFScalarType distance = input->GetPixelValueByIndex(pixel); //TODO: float array zugriff

      mitk::ToFProcessingCommon::ToFPoint3D cartesianCoordinates =
          mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,m_CameraModel->GetFocalLength(),m_CameraModel->GetInterPixelDistance(),m_CameraModel->GetPrincipalPoint());

      //TODO: why epsilon here and what value should it have?
//      if (cartesianCoordinates[2] == 0)
      if (distance<=mitk::eps)
      {
        isPointValid[pointCount] = false;
      }
      else
      {
        isPointValid[pointCount] = true;
      
        points->InsertPoint(pixelID, cartesianCoordinates.GetDataPointer());

        if((i >= 1) && (j >= 1))
        {
          vtkIdType xy = i+j*xDimension;
          vtkIdType x_1y = i-1+j*xDimension;
          vtkIdType xy_1 = i+(j-1)*xDimension;
          vtkIdType x_1y_1 = (i-1)+(j-1)*xDimension;

          if (isPointValid[xy]&&isPointValid[x_1y]&&isPointValid[x_1y_1]&&isPointValid[xy_1]) // check if points of cell are valid
          {
            polys->InsertNextCell(3);
            polys->InsertCellPoint(xy);
            polys->InsertCellPoint(x_1y);
            polys->InsertCellPoint(x_1y_1);

            polys->InsertNextCell(3);
            polys->InsertCellPoint(xy);
            polys->InsertCellPoint(x_1y_1);
            polys->InsertCellPoint(xy_1);
          }
        }

        if (scalarFloatData)
        {          
          scalarArray->InsertTuple1(pixelID, scalarFloatData[pixel[0]+pixel[1]*xDimension]);
        }
        if (this->m_TextureImageHeight > 0.0 && this->m_TextureImageWidth > 0.0)
        {

          float xNorm = (((float)pixel[0])/xDimension)*textureScaleCorrection1 + textureScaleCorrection2 ; // correct video texture scale 640 * 480!! 
          float yNorm = 1.0 - ((float)pixel[1])/yDimension; //flip y-axis
          textureCoords->InsertTuple2(pixelID, xNorm, yNorm);
        }
      }
      pointCount++;
    }
  }

  vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
  mesh->SetPoints(points);
  mesh->SetPolys(polys);
  if (scalarArray->GetNumberOfTuples()>0)
  {
    mesh->GetPointData()->SetScalars(scalarArray);
    if (this->m_TextureImageHeight > 0.0 && this->m_TextureImageWidth > 0.0)
    {
      mesh->GetPointData()->SetTCoords(textureCoords);
    }
  }
  output->SetVtkPolyData(mesh);
}

void mitk::ToFDistanceImageToSurfaceFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
    if (this->GetOutput(idx) == NULL)
    {
    DataObjectPointer newOutput = this->MakeOutput(idx);
    this->SetNthOutput(idx, newOutput);
  }
  this->Modified();
}

void mitk::ToFDistanceImageToSurfaceFilter::GenerateOutputInformation()
{
  this->GetOutput();
  itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::ToFDistanceImageToSurfaceFilter::SetScalarImage(IplImage* iplScalarImage)
{
  this->m_IplScalarImage = iplScalarImage;
  this->Modified();
}

IplImage* mitk::ToFDistanceImageToSurfaceFilter::GetScalarImage()
{
  return this->m_IplScalarImage;
}

void mitk::ToFDistanceImageToSurfaceFilter::SetTextureImageWidth(int width)
{
  this->m_TextureImageWidth = width;
}

void mitk::ToFDistanceImageToSurfaceFilter::SetTextureImageHeight(int height)
{
  this->m_TextureImageHeight = height;
}
