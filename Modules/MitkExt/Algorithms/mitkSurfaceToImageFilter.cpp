/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSurfaceToImageFilter.h"
#include "mitkTimeHelper.h"

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
     (inputImage->GetTimeSlicedGeometry() == NULL)) return;

  output->Initialize(inputImage->GetPixelType(), *inputImage->GetTimeSlicedGeometry());

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
  const mitk::TimeSlicedGeometry *surfaceTimeGeometry = GetInput()->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *imageTimeGeometry = GetImage()->GetTimeSlicedGeometry();

  // Convert time step from image time-frame to surface time-frame
  int surfaceTimeStep = surfaceTimeGeometry->TimeStepToTimeStep( imageTimeGeometry, time );
  
  vtkPolyData * polydata = ( (mitk::Surface*)GetInput() )->GetVtkPolyData( surfaceTimeStep );

  vtkTransformPolyDataFilter * move=vtkTransformPolyDataFilter::New();
  move->SetInput(polydata);
  move->ReleaseDataFlagOn();

  vtkTransform *transform=vtkTransform::New();
  Geometry3D* geometry = surfaceTimeGeometry->GetGeometry3D( surfaceTimeStep );
  geometry->TransferItkToVtkTransform();
  transform->PostMultiply();
  transform->Concatenate(geometry->GetVtkTransform()->GetMatrix());
  // take image geometry into account. vtk-Image information will be changed to unit spacing and zero origin below.
  Geometry3D* imageGeometry = imageTimeGeometry->GetGeometry3D(time);
  imageGeometry->TransferItkToVtkTransform();
  transform->Concatenate(imageGeometry->GetVtkTransform()->GetLinearInverse());
  move->SetTransform(transform);

  vtkPolyDataNormals * normalsFilter = vtkPolyDataNormals::New();
  normalsFilter->SetFeatureAngle(50);
  normalsFilter->SetConsistency(1);
  normalsFilter->SetSplitting(1);
  normalsFilter->SetFlipNormals(0);
  normalsFilter->ReleaseDataFlagOn();

  normalsFilter->SetInput( move->GetOutput() );
  move->Delete();

  vtkPolyDataToImageStencil * surfaceConverter = vtkPolyDataToImageStencil::New();
  surfaceConverter->SetTolerance( 0.0 );
  surfaceConverter->ReleaseDataFlagOn();

  surfaceConverter->SetInput( normalsFilter->GetOutput() );
  normalsFilter->Delete();

  vtkImageData *image = const_cast< mitk::Image * >(this->GetImage())->GetVtkImageData( time );
  
  // Create stencil and use numerical minimum of pixel type as background value
  vtkImageStencil * stencil = vtkImageStencil::New();
  stencil->SetInput( image );
  stencil->ReverseStencilOff();
  stencil->ReleaseDataFlagOn();
  stencil->SetStencil( surfaceConverter->GetOutput() );
  surfaceConverter->Delete();

  if (m_MakeOutputBinary)
  {
    stencil->SetBackgroundValue( image->GetScalarTypeMin() );

    vtkImageThreshold * threshold = vtkImageThreshold::New();
    threshold->SetInput( stencil->GetOutput() );
    threshold->ThresholdByLower( image->GetScalarTypeMin() );
    threshold->ReplaceInOn();
    threshold->ReplaceOutOn();
    threshold->SetInValue( 0 );
    threshold->SetOutValue( 1 );
    threshold->Update();

    mitk::Image::Pointer output = this->GetOutput();
    output->SetVolume( threshold->GetOutput()->GetScalarPointer(), time );

    threshold->Delete();
  }
  else
  {
    stencil->SetBackgroundValue( m_BackgroundValue );
    stencil->Update();

    mitk::Image::Pointer output = this->GetOutput();
    output->SetVolume( stencil->GetOutput()->GetScalarPointer(), time );
    LOG_INFO << "stencil ref count: " << stencil->GetReferenceCount() << std::endl;
  }
  stencil->Delete();
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
