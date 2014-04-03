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

#include "mitkSurfaceToImageFilter.h"
#include "mitkTimeHelper.h"
#include "mitkImageWriteAccessor.h"

#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkLinearTransform.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkImageThreshold.h>
#include <vtkImageMathematics.h>
#include <vtkPolyDataNormals.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>


mitk::SurfaceToImageFilter::SurfaceToImageFilter()
: m_MakeOutputBinary( false ),
  m_BackgroundValue( -10000 )
{
}

mitk::SurfaceToImageFilter::~SurfaceToImageFilter()
{
}

void mitk::SurfaceToImageFilter::GenerateInputRequestedRegion()
{
  mitk::Image* output = this->GetOutput();
  if((output->IsInitialized()==false) )
    return;

  GenerateTimeInInputRegion(output, const_cast< mitk::Image * > ( this->GetImage() ));
}

void mitk::SurfaceToImageFilter::GenerateOutputInformation()
{
  mitk::Image *inputImage = (mitk::Image*)this->GetImage();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if((inputImage == NULL) ||
     (inputImage->IsInitialized() == false) ||
     (inputImage->GetTimeGeometry() == NULL)) return;

  if (m_MakeOutputBinary)
    output->Initialize(mitk::MakeScalarPixelType<unsigned char>() , *inputImage->GetTimeGeometry());
  else
    output->Initialize(inputImage->GetPixelType(), *inputImage->GetTimeGeometry());

  output->SetPropertyList(inputImage->GetPropertyList()->Clone());
}

void mitk::SurfaceToImageFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetImage();
  mitk::Image::Pointer output = this->GetOutput();

  if(inputImage.IsNull())
    return;

  if(output->IsInitialized()==false )
    return;

  mitk::Image::RegionType outputRegion = output->GetRequestedRegion();

  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3);

  if ( tmax > 0)
  {
    int t;
    for(t=tstart;t<tmax;++t)
    {
      Stencil3DImage( t );
    }
  }
  else
  {
    Stencil3DImage( 0 );
  }
}

void mitk::SurfaceToImageFilter::Stencil3DImage(int time)
{
  const mitk::TimeGeometry *surfaceTimeGeometry = GetInput()->GetTimeGeometry();
  const mitk::TimeGeometry *imageTimeGeometry = GetImage()->GetTimeGeometry();

  // Convert time step from image time-frame to surface time-frame
  mitk::TimePointType matchingTimePoint = imageTimeGeometry->TimeStepToTimePoint(time);
  mitk::TimeStepType surfaceTimeStep = surfaceTimeGeometry->TimePointToTimeStep(matchingTimePoint);

  vtkPolyData * polydata = ( (mitk::Surface*)GetInput() )->GetVtkPolyData( surfaceTimeStep );

  vtkSmartPointer<vtkTransformPolyDataFilter> move=vtkTransformPolyDataFilter::New();
  move->SetInputData(polydata);
  move->ReleaseDataFlagOn();

  vtkSmartPointer<vtkTransform> transform=vtkTransform::New();
  BaseGeometry* geometry = surfaceTimeGeometry->GetGeometryForTimeStep( surfaceTimeStep );
  geometry->TransferItkToVtkTransform();
  transform->PostMultiply();
  transform->Concatenate(geometry->GetVtkTransform()->GetMatrix());
  // take image geometry into account. vtk-Image information will be changed to unit spacing and zero origin below.
  BaseGeometry* imageGeometry = imageTimeGeometry->GetGeometryForTimeStep(time);
  imageGeometry->TransferItkToVtkTransform();
  transform->Concatenate(imageGeometry->GetVtkTransform()->GetLinearInverse());
  move->SetTransform(transform);

  vtkSmartPointer<vtkPolyDataNormals> normalsFilter = vtkPolyDataNormals::New();
  normalsFilter->SetFeatureAngle(50);
  normalsFilter->SetConsistency(1);
  normalsFilter->SetSplitting(1);
  normalsFilter->SetFlipNormals(0);
  normalsFilter->ReleaseDataFlagOn();

  normalsFilter->SetInputConnection(move->GetOutputPort() );

  vtkSmartPointer<vtkPolyDataToImageStencil> surfaceConverter = vtkPolyDataToImageStencil::New();
  surfaceConverter->SetTolerance( 0.0 );
  surfaceConverter->ReleaseDataFlagOn();

  surfaceConverter->SetInputConnection( normalsFilter->GetOutputPort() );

  mitk::Image::Pointer binaryImage = mitk::Image::New();

  if (m_MakeOutputBinary)
  {
    binaryImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), *this->GetImage()->GetTimeGeometry());

    unsigned int size = sizeof(unsigned char);
    for (unsigned int i = 0; i < binaryImage->GetDimension(); ++i)
      size *= binaryImage->GetDimension(i);

    mitk::ImageWriteAccessor accessor( binaryImage );
    memset( accessor.GetData(), 1, size );
  }

  vtkImageData *image = m_MakeOutputBinary
    ? binaryImage->GetVtkImageData(time)
    : const_cast<mitk::Image *>(this->GetImage())->GetVtkImageData(time);

  // Create stencil and use numerical minimum of pixel type as background value
  vtkSmartPointer<vtkImageStencil> stencil = vtkImageStencil::New();
  stencil->SetInputData(image);
  stencil->ReverseStencilOff();
  stencil->ReleaseDataFlagOn();
  stencil->SetStencilConnection(surfaceConverter->GetOutputPort());

  stencil->SetBackgroundValue(m_MakeOutputBinary ? 0 : m_BackgroundValue);
  stencil->Update();

  mitk::Image::Pointer output = this->GetOutput();
  output->SetVolume( stencil->GetOutput()->GetScalarPointer(), time );
  MITK_INFO << "stencil ref count: " << stencil->GetReferenceCount() << std::endl;
}


const mitk::Surface *mitk::SurfaceToImageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Surface * >
    ( this->ProcessObject::GetInput(0) );
}

void mitk::SurfaceToImageFilter::SetInput(const mitk::Surface *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0,
    const_cast< mitk::Surface * >( input ) );
}

void mitk::SurfaceToImageFilter::SetImage(const mitk::Image *source)
{
  this->ProcessObject::SetNthInput( 1, const_cast< mitk::Image * >( source ) );
}

const mitk::Image *mitk::SurfaceToImageFilter::GetImage(void)
{
  return static_cast< const mitk::Image * >(this->ProcessObject::GetInput(1));
}
