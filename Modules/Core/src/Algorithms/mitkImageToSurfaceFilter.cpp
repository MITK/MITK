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

#include <mitkImageToSurfaceFilter.h>
#include "mitkException.h"
#include "mitkImageVtkAccessor.h"

#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkDecimatePro.h>
#include <vtkImageChangeInformation.h>
#include <vtkLinearTransform.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkQuadricDecimation.h>

#include <vtkSmartPointer.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>

mitk::ImageToSurfaceFilter* mitk::ImageToSurfaceFilter::m_CurrentlyProgressingBuilder = nullptr;
float mitk::ImageToSurfaceFilter::m_ProgressWeight = 0.f;
float mitk::ImageToSurfaceFilter::m_CurrentProgress = 0.f;

mitk::ImageToSurfaceFilter::ImageToSurfaceFilter() :
  m_Smooth(false),
  m_Decimate(NoDecimation),
  m_Threshold(1.0),
  m_TargetReduction(0.95f),
  m_SmoothIteration(50),
  m_SmoothRelaxation(0.1)
{
  m_VtkProgressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  m_VtkProgressCallback->SetCallback(mitk::ImageToSurfaceFilter::vtkOnProgress);

  m_VtkEndCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  m_VtkEndCallback->SetCallback(mitk::ImageToSurfaceFilter::vtkOnEnd);
}

mitk::ImageToSurfaceFilter::~ImageToSurfaceFilter()
{
}

void mitk::ImageToSurfaceFilter::CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold)
{
  vtkImageChangeInformation *indexCoordinatesImageFilter = vtkImageChangeInformation::New();
  indexCoordinatesImageFilter->SetInputData(vtkimage);
  indexCoordinatesImageFilter->SetOutputOrigin(0.0,0.0,0.0);

  //MarchingCube -->create Surface
  vtkSmartPointer<vtkMarchingCubes> skinExtractor = vtkSmartPointer<vtkMarchingCubes>::New();
  skinExtractor->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
  skinExtractor->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
  skinExtractor->ComputeScalarsOff();
  skinExtractor->SetInputConnection(indexCoordinatesImageFilter->GetOutputPort());//RC++
  indexCoordinatesImageFilter->Delete();
  skinExtractor->SetValue(0, threshold);

  vtkPolyData *polydata;
  skinExtractor->Update();
  polydata = skinExtractor->GetOutput();
  polydata->Register(nullptr);//RC++

  if (m_Smooth)
  {
    vtkSmoothPolyDataFilter *smoother = vtkSmoothPolyDataFilter::New();
    //read poly1 (poly1 can be the original polygon, or the decimated polygon)
    smoother->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
    smoother->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
    smoother->SetInputConnection(skinExtractor->GetOutputPort());//RC++
    smoother->SetNumberOfIterations( m_SmoothIteration );
    smoother->SetRelaxationFactor( m_SmoothRelaxation );
    smoother->SetFeatureAngle( 60 );
    smoother->FeatureEdgeSmoothingOff();
    smoother->BoundarySmoothingOff();
    smoother->SetConvergence( 0 );
    smoother->Update();

    polydata->Delete();//RC--
    polydata = smoother->GetOutput();
    polydata->Register(nullptr);//RC++
    smoother->Delete();
  }

  //decimate = to reduce number of polygons
  if(m_Decimate==DecimatePro)
  {
    vtkDecimatePro *decimate = vtkDecimatePro::New();
    decimate->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
    decimate->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
    decimate->SplittingOff();
    decimate->SetErrorIsAbsolute(5);
    decimate->SetFeatureAngle(30);
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetDegree(10); //std-value is 25!

    decimate->SetInputData(polydata);//RC++
    decimate->SetTargetReduction(m_TargetReduction);
    decimate->SetMaximumError(0.002);
    decimate->Update();

    polydata->Delete();//RC--
    polydata = decimate->GetOutput();
    polydata->Register(nullptr);//RC++
    decimate->Delete();
  }
  else if (m_Decimate==QuadricDecimation)
  {
    vtkQuadricDecimation* decimate = vtkQuadricDecimation::New();
    decimate->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
    decimate->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
    decimate->SetTargetReduction(m_TargetReduction);

    decimate->SetInputData(polydata);
    decimate->Update();
    polydata->Delete();
    polydata = decimate->GetOutput();
    polydata->Register(nullptr);
    decimate->Delete();
  }

  if(polydata->GetNumberOfPoints() > 0)
  {
    mitk::Vector3D spacing = GetInput()->GetGeometry(time)->GetSpacing();

    vtkPoints * points = polydata->GetPoints();
    vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
    GetInput()->GetGeometry(time)->GetVtkTransform()->GetMatrix(vtkmatrix);
    double (*matrix)[4] = vtkmatrix->Element;

    unsigned int i,j;
    for(i=0;i<3;++i)
      for(j=0;j<3;++j)
        matrix[i][j]/=spacing[j];

    unsigned int n = points->GetNumberOfPoints();
    double point[3];

    for (i = 0; i < n; i++)
    {
      points->GetPoint(i, point);
      mitkVtkLinearTransformPoint(matrix,point,point);
      points->SetPoint(i, point);
    }
    vtkmatrix->Delete();
  }

  // determine point_data normals for the poly data points.
  vtkSmartPointer<vtkPolyDataNormals> normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsGenerator->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
  normalsGenerator->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
  normalsGenerator->SetInputData( polydata );
  normalsGenerator->AutoOrientNormalsOn();
  normalsGenerator->FlipNormalsOff();
  normalsGenerator->ConsistencyOn();
  normalsGenerator->ComputeCellNormalsOff();
  normalsGenerator->SplittingOff();

  vtkSmartPointer<vtkCleanPolyData> cleanPolyDataFilter = vtkSmartPointer<vtkCleanPolyData>::New();
  cleanPolyDataFilter->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
  cleanPolyDataFilter->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
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
  mitk::Image * image        =  (mitk::Image*)GetInput();
  if(image == nullptr || !image->IsInitialized())
    mitkThrow() << "No input image set, please set an valid input image!";

  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();

  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3); //GetSize()==1 - will aber 0 haben, wenn nicht zeitaufgeloest

  m_ProgressWeight = 1.f / (1 // Skin Extractor
    + (float)m_Smooth
    + (float)(m_Decimate == DecimatePro) + (float)(m_Decimate == QuadricDecimation)
    + 1 // Normals
    + 1); // Clean poly data

  m_CurrentProgress = 0.f;
  m_CurrentlyProgressingBuilder = this;

  int t;
  for( t=tstart; t < tmax; ++t)
  {
    Image::Pointer imagePointer = image;
    ImageVtkAccessor accessor(imagePointer);
    ImageAccessLock lock(&accessor);
    vtkImageData *vtkimagedata = accessor.getVtkImageData(t);
    CreateSurface(t,vtkimagedata,surface,m_Threshold);
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
  this->ProcessObject::SetNthInput(0, const_cast< mitk::Image * >( image ) );
}


const mitk::Image *mitk::ImageToSurfaceFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return nullptr;
  }

  return static_cast<const mitk::Image * >
    ( this->ProcessObject::GetInput(0) );
}


void mitk::ImageToSurfaceFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer inputImage  =(mitk::Image*) this->GetInput();

  //mitk::Image *inputImage = (mitk::Image*)this->GetImage();
  mitk::Surface::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(inputImage.IsNull()) return;

  //Set Data
}

void mitk::ImageToSurfaceFilter::vtkOnProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
  void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkPolyDataAlgorithm* filter = static_cast<vtkPolyDataAlgorithm*>(caller);
  if (filter == nullptr) {
    std::cout << "filter bad =c";
  }

  m_CurrentlyProgressingBuilder->UpdateProgress(m_CurrentProgress + filter->GetProgress() * m_ProgressWeight);

  if (m_CurrentlyProgressingBuilder->GetAbortGenerateData()) {
    throw itk::ProcessAborted();
  }
}

void mitk::ImageToSurfaceFilter::vtkOnEnd(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
  void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  m_CurrentProgress += m_ProgressWeight;
}
