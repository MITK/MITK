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

#include "mitkException.h"
#include <mitkImageToSurfaceFilter.h>
#include <vtkDecimatePro.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkQuadricDecimation.h>

#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>

#include "mitkProgressBar.h"

mitk::ImageToSurfaceFilter::ImageToSurfaceFilter()
  : m_Smooth(false),
    m_Decimate(NoDecimation),
    m_Threshold(1.0),
    m_TargetReduction(0.95f),
    m_SmoothIteration(50),
    m_SmoothRelaxation(0.1)
{
}

mitk::ImageToSurfaceFilter::~ImageToSurfaceFilter()
{
}

void mitk::ImageToSurfaceFilter::CreateSurface(int time,
                                               vtkImageData *vtkimage,
                                               mitk::Surface *surface,
                                               const ScalarType threshold)
{
  vtkImageChangeInformation *indexCoordinatesImageFilter = vtkImageChangeInformation::New();
  indexCoordinatesImageFilter->SetInputData(vtkimage);
  indexCoordinatesImageFilter->SetOutputOrigin(0.0, 0.0, 0.0);

  // MarchingCube -->create Surface
  vtkSmartPointer<vtkMarchingCubes> skinExtractor = vtkSmartPointer<vtkMarchingCubes>::New();
  skinExtractor->ComputeScalarsOff();
  skinExtractor->SetInputConnection(indexCoordinatesImageFilter->GetOutputPort()); // RC++
  indexCoordinatesImageFilter->Delete();
  skinExtractor->SetValue(0, threshold);

  vtkPolyData *polydata;
  skinExtractor->Update();
  polydata = skinExtractor->GetOutput();
  polydata->Register(nullptr); // RC++

  if (m_Smooth)
  {
    vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
    // read poly1 (poly1 can be the original polygon, or the decimated polygon)
    smoother->SetInputConnection(skinExtractor->GetOutputPort()); // RC++
    smoother->SetNumberOfIterations(m_SmoothIteration);
    smoother->SetRelaxationFactor(m_SmoothRelaxation);
    smoother->SetFeatureAngle(60);
    smoother->FeatureEdgeSmoothingOff();
    smoother->BoundarySmoothingOff();
    smoother->SetConvergence(0);
    smoother->Update();

    polydata->Delete(); // RC--
    polydata = smoother->GetOutput();
    polydata->Register(nullptr); // RC++
    smoother->Delete();
  }
  ProgressBar::GetInstance()->Progress();

  // decimate = to reduce number of polygons
  if (m_Decimate == DecimatePro)
  {
    vtkDecimatePro *decimate = vtkDecimatePro::New();
    decimate->SplittingOff();
    decimate->SetErrorIsAbsolute(5);
    decimate->SetFeatureAngle(30);
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetDegree(10); // std-value is 25!

    decimate->SetInputData(polydata); // RC++
    decimate->SetTargetReduction(m_TargetReduction);
    decimate->SetMaximumError(0.002);
    decimate->Update();

    polydata->Delete(); // RC--
    polydata = decimate->GetOutput();
    polydata->Register(nullptr); // RC++
    decimate->Delete();
  }
  else if (m_Decimate == QuadricDecimation)
  {
    vtkQuadricDecimation *decimate = vtkQuadricDecimation::New();
    decimate->SetTargetReduction(m_TargetReduction);

    decimate->SetInputData(polydata);
    decimate->Update();
    polydata->Delete();
    polydata = decimate->GetOutput();
    polydata->Register(nullptr);
    decimate->Delete();
  }

  ProgressBar::GetInstance()->Progress();

  if (polydata->GetNumberOfPoints() > 0)
  {
    mitk::Vector3D spacing = GetInput()->GetGeometry(time)->GetSpacing();

    vtkPoints *points = polydata->GetPoints();
    vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
    GetInput()->GetGeometry(time)->GetVtkTransform()->GetMatrix(vtkmatrix);
    double(*matrix)[4] = vtkmatrix->Element;

    unsigned int i, j;
    for (i = 0; i < 3; ++i)
      for (j = 0; j < 3; ++j)
        matrix[i][j] /= spacing[j];

    unsigned int n = points->GetNumberOfPoints();
    double point[3];

    for (i = 0; i < n; i++)
    {
      points->GetPoint(i, point);
      mitkVtkLinearTransformPoint(matrix, point, point);
      points->SetPoint(i, point);
    }
    vtkmatrix->Delete();
  }
  ProgressBar::GetInstance()->Progress();

  // determine point_data normals for the poly data points.
  vtkSmartPointer<vtkPolyDataNormals> normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsGenerator->SetInputData(polydata);
  normalsGenerator->FlipNormalsOn();

  vtkSmartPointer<vtkCleanPolyData> cleanPolyDataFilter = vtkSmartPointer<vtkCleanPolyData>::New();
  cleanPolyDataFilter->SetInputConnection(normalsGenerator->GetOutputPort());
  cleanPolyDataFilter->PieceInvariantOff();
  cleanPolyDataFilter->ConvertLinesToPointsOff();
  cleanPolyDataFilter->ConvertPolysToLinesOff();
  cleanPolyDataFilter->ConvertStripsToPolysOff();
  cleanPolyDataFilter->PointMergingOn();
  cleanPolyDataFilter->Update();

  surface->SetVtkPolyData(cleanPolyDataFilter->GetOutput(), time);
  polydata->UnRegister(nullptr);
}

void mitk::ImageToSurfaceFilter::GenerateData()
{
  mitk::Surface *surface = this->GetOutput();
  auto *image = (mitk::Image *)GetInput();
  if (image == nullptr || !image->IsInitialized())
    mitkThrow() << "No input image set, please set an valid input image!";

  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();

  int tstart = outputRegion.GetIndex(3);
  int tmax = tstart + outputRegion.GetSize(3); // GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgeloest

  if ((tmax - tstart) > 0)
  {
    ProgressBar::GetInstance()->AddStepsToDo(4 * (tmax - tstart));
  }

  int t;
  for (t = tstart; t < tmax; ++t)
  {
    vtkImageData *vtkimagedata = image->GetVtkImageData(t);
    CreateSurface(t, vtkimagedata, surface, m_Threshold);
    ProgressBar::GetInstance()->Progress();
  }
}

void mitk::ImageToSurfaceFilter::SetSmoothIteration(int smoothIteration)
{
  m_SmoothIteration = smoothIteration;
}

void mitk::ImageToSurfaceFilter::SetSmoothRelaxation(float smoothRelaxation)
{
  m_SmoothRelaxation = smoothRelaxation;
}

void mitk::ImageToSurfaceFilter::SetInput(const mitk::Image *image)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<mitk::Image *>(image));
}

const mitk::Image *mitk::ImageToSurfaceFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return nullptr;
  }

  return static_cast<const mitk::Image *>(this->ProcessObject::GetInput(0));
}

void mitk::ImageToSurfaceFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer inputImage = (mitk::Image *)this->GetInput();

  // mitk::Image *inputImage = (mitk::Image*)this->GetImage();
  mitk::Surface::Pointer output = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");

  if (inputImage.IsNull())
    return;

  // Set Data
}
