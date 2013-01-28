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

#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkInstantiateAccessFunctions.h>
#include <mitkSurface.h>

#include <itkImage.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkSmartPointer.h>
#include <vtkIdList.h>

#include <math.h>

mitk::ToFDistanceImageToSurfaceFilter::ToFDistanceImageToSurfaceFilter() :
  m_IplScalarImage(NULL), m_CameraIntrinsics(), m_TextureImageWidth(0), m_TextureImageHeight(0), m_InterPixelDistance(), m_TextureIndex(0)
{
  m_InterPixelDistance.Fill(0.045);
  m_CameraIntrinsics = mitk::CameraIntrinsics::New();
  m_CameraIntrinsics->SetFocalLength(295.78960196187319,296.1255427948447);
  m_CameraIntrinsics->SetFocalLength(5.9421434211923247e+02,5.9104053696870778e+02);
  m_CameraIntrinsics->SetPrincipalPoint(3.3930780975300314e+02,2.4273913761751615e+02);
  m_CameraIntrinsics->SetDistorsionCoeffs(-0.36874385358645773f,-0.14339503290129013,0.0033210108720361795,-0.004277703352074105);
  m_ReconstructionMode = WithInterPixelDistance;
}

mitk::ToFDistanceImageToSurfaceFilter::~ToFDistanceImageToSurfaceFilter()
{
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput( Image* distanceImage, mitk::CameraIntrinsics::Pointer cameraIntrinsics )
{
  this->SetCameraIntrinsics(cameraIntrinsics);
  this->SetInput(0,distanceImage);
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput( unsigned int idx,  Image* distanceImage, mitk::CameraIntrinsics::Pointer cameraIntrinsics )
{
  this->SetCameraIntrinsics(cameraIntrinsics);
  this->SetInput(idx,distanceImage);
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput(  mitk::Image* distanceImage )
{
  this->SetInput(0,distanceImage);
}

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
  {
    mitkThrow() << "No input given for ToFDistanceImageToSurfaceFilter";
  }
  return static_cast< mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ToFDistanceImageToSurfaceFilter::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();
  assert(output);
  mitk::Image::Pointer input = this->GetInput();
  assert(input);
  // mesh points
  int xDimension = input->GetDimension(0);
  int yDimension = input->GetDimension(1);
  unsigned int size = xDimension*yDimension; //size of the image-array
  std::vector<bool> isPointValid;
  isPointValid.resize(size);
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkFloatArray> scalarArray = vtkSmartPointer<vtkFloatArray>::New();
  vtkSmartPointer<vtkFloatArray> textureCoords = vtkSmartPointer<vtkFloatArray>::New();
  textureCoords->SetNumberOfComponents(2);
  textureCoords->Allocate(size);

  //Make a vtkIdList to save the ID's of the polyData corresponding to the image
  //pixel ID's. See below for more documentation.
  vtkSmartPointer<vtkIdList> vertexIdList = vtkSmartPointer<vtkIdList>::New();
  //Allocate the object once else it would automatically allocate new memory
  //for every vertex and perform a copy which is expensive.
  vertexIdList->Allocate(size);

  float* scalarFloatData = NULL;

  if (this->m_IplScalarImage) // if scalar image is defined use it for texturing
  {
    scalarFloatData = (float*)this->m_IplScalarImage->imageData;
  }
  else if (this->GetInput(m_TextureIndex)) // otherwise use intensity image (input(2))
  {
    scalarFloatData = (float*)this->GetInput(m_TextureIndex)->GetData();
  }

  float* inputFloatData = (float*)(input->GetSliceData(0, 0, 0)->GetData());
  //calculate world coordinates
  mitk::ToFProcessingCommon::ToFPoint2D focalLengthInPixelUnits;
  mitk::ToFProcessingCommon::ToFScalarType focalLengthInMm;
  if((m_ReconstructionMode == WithOutInterPixelDistance) || (m_ReconstructionMode == Kinect))
  {
    focalLengthInPixelUnits[0] = m_CameraIntrinsics->GetFocalLengthX();
    focalLengthInPixelUnits[1] = m_CameraIntrinsics->GetFocalLengthY();
  }
  else if( m_ReconstructionMode == WithInterPixelDistance)
  {
    //convert focallength from pixel to mm
    focalLengthInMm = (m_CameraIntrinsics->GetFocalLengthX()*m_InterPixelDistance[0]+m_CameraIntrinsics->GetFocalLengthY()*m_InterPixelDistance[1])/2.0;
  }

  mitk::ToFProcessingCommon::ToFPoint2D principalPoint;
  principalPoint[0] = m_CameraIntrinsics->GetPrincipalPointX();
  principalPoint[1] = m_CameraIntrinsics->GetPrincipalPointY();

  mitk::Point3D origin = input->GetGeometry()->GetOrigin();

  for (int j=0; j<yDimension; j++)
  {
    for (int i=0; i<xDimension; i++)
    {
      unsigned int pixelID = i+j*xDimension;

      mitk::ToFProcessingCommon::ToFScalarType distance = (double)inputFloatData[pixelID];

      mitk::ToFProcessingCommon::ToFPoint3D cartesianCoordinates;
      switch (m_ReconstructionMode)
      {
      case WithOutInterPixelDistance:
      {
        cartesianCoordinates = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i+origin[0],j+origin[1],distance,focalLengthInPixelUnits,principalPoint);
        break;
      }
      case WithInterPixelDistance:
      {
        cartesianCoordinates = mitk::ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(i+origin[0],j+origin[1],distance,focalLengthInMm,m_InterPixelDistance,principalPoint);
        break;
      }
      case Kinect:
      {
        cartesianCoordinates = mitk::ToFProcessingCommon::KinectIndexToCartesianCoordinates(i+origin[0],j+origin[1],distance,focalLengthInPixelUnits,principalPoint);
        break;
      }
      default:
      {
        MITK_ERROR << "Incorrect reconstruction mode!";
      }
      }
      //Epsilon here, because we may have small float values like 0.00000001 which in fact represents 0.
      if (distance<=mitk::eps)
      {
        isPointValid[pixelID] = false;
      }
      else
      {
        isPointValid[pixelID] = true;
        //VTK would insert empty points into the polydata if we use
        //points->InsertPoint(pixelID, cartesianCoordinates.GetDataPointer()).
        //If we use points->InsertNextPoint(...) instead, the ID's do not
        //correspond to the image pixel ID's. Thus, we have to save them
        //in the vertexIdList.
        vertexIdList->InsertId(pixelID, points->InsertNextPoint(cartesianCoordinates.GetDataPointer()));

        if((i >= 1) && (j >= 1))
        {
          //This little piece of art explains the ID's:
          //
          // P(x_1y_1)---P(xy_1)
          // |           |
          // |           |
          // |           |
          // P(x_1y)-----P(xy)
          //
          //We can only start triangulation if we are at vertex (1,1),
          //because we need the other 3 vertices near this one.
          //To go one pixel line back in the image array, we have to
          //subtract 1x xDimension.
          vtkIdType xy = pixelID;
          vtkIdType x_1y = pixelID-1;
          vtkIdType xy_1 = pixelID-xDimension;
          vtkIdType x_1y_1 = xy_1-1;

          //Find the corresponding vertex ID's in the saved vertexIdList:
          vtkIdType xyV = vertexIdList->GetId(xy);
          vtkIdType x_1yV = vertexIdList->GetId(x_1y);
          vtkIdType xy_1V = vertexIdList->GetId(xy_1);
          vtkIdType x_1y_1V = vertexIdList->GetId(x_1y_1);

          if (isPointValid[xy]&&isPointValid[x_1y]&&isPointValid[x_1y_1]&&isPointValid[xy_1]) // check if points of cell are valid
          {
            polys->InsertNextCell(3);
            polys->InsertCellPoint(x_1yV);
            polys->InsertCellPoint(xyV);
            polys->InsertCellPoint(x_1y_1V);

            polys->InsertNextCell(3);
            polys->InsertCellPoint(x_1y_1V);
            polys->InsertCellPoint(xyV);
            polys->InsertCellPoint(xy_1V);
          }
        }
        //Scalar values are necessary for mapping colors/texture onto the surface
        if (scalarFloatData)
        {
          scalarArray->InsertTuple1(vertexIdList->GetId(pixelID), scalarFloatData[pixelID]);
        }
        //These Texture Coordinates will map color pixel and vertices 1:1 (e.g. for Kinect).
        float xNorm = (((float)i)/xDimension);// correct video texture scale for kinect
        float yNorm = ((float)j)/yDimension; //don't flip. we don't need to flip.
        textureCoords->InsertTuple2(vertexIdList->GetId(pixelID), xNorm, yNorm);
      }
    }
  }

  vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
  mesh->SetPoints(points);
  mesh->SetPolys(polys);
  //Pass the scalars to the polydata (if they were set).
  if (scalarArray->GetNumberOfTuples()>0)
  {
    mesh->GetPointData()->SetScalars(scalarArray);
  }
  //Pass the TextureCoords to the polydata anyway (to save them).
  mesh->GetPointData()->SetTCoords(textureCoords);
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
