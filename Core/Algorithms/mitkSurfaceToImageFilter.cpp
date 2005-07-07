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
#include <vtkTriangleFilter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkImageThreshold.h>
#include <vtkImageMathematics.h>
#include <vtkImageCast.h>
#include <vtkPolyDataNormals.h>

#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkImageChangeInformation.h>

mitk::SurfaceToImageFilter::SurfaceToImageFilter()
{
  m_MakeOutputBinaryOn = false;
  m_BackgroundValue = -10000;
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
  mitk::Surface::ConstPointer input  = this->GetInput();
  mitk::Image *inputImage = (mitk::Image*)this->GetImage();
  mitk::Image::Pointer output = this->GetOutput();

    // set input-requested-region, because we access it later in
  // GenerateInputRequestedRegion (there we just set the time)
//  inputImage->SetRequestedRegion( inputImage->GetLargestPossibleRegion() );

 
  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull()) return;

  unsigned int dimension = inputImage->GetDimension();
  unsigned int *dimensions = new unsigned int [dimension];
  memcpy(dimensions, inputImage->GetDimensions(), dimension*sizeof(unsigned int));
  mitk::PixelType pixelType;
  pixelType.Initialize(typeid(int));
  output->Initialize( pixelType, dimension, dimensions);
  delete [] dimensions;

  // set the spacing
  mitk::Vector3D spacing = inputImage->GetSlicedGeometry()->GetSpacing();
  output->SetSpacing(spacing);
  output->SetRequestedRegionToLargestPossibleRegion();

  // Position the output Image to match the corresponding region of the input image
  mitk::SlicedGeometry3D* slicedGeometry = output->GetSlicedGeometry();
  const mitk::SlicedData::IndexType& start = inputImage->GetLargestPossibleRegion().GetIndex();
  mitk::Vector3D vector; vtk2itk(start, vector);
  slicedGeometry->Translate(vector);

  mitk::TimeSlicedGeometry* timeSlicedGeometry = output->GetTimeSlicedGeometry();
  timeSlicedGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
  timeSlicedGeometry->CopyTimes(inputImage->GetTimeSlicedGeometry());
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
  
  int surfaceTimeStep = (int) surfaceTimeGeometry->TimeStepToMS( (int) (imageTimeGeometry->TimeStepToMS(time) ) );
  
  vtkPolyData * polydata = ( (mitk::Surface*)GetInput() )->GetVtkPolyData( surfaceTimeStep );

  vtkTransformPolyDataFilter * move=vtkTransformPolyDataFilter::New();
  move->SetInput(polydata);
  move->ReleaseDataFlagOn();

  vtkTransform *transform=vtkTransform::New();
  surfaceTimeGeometry->GetGeometry3D( surfaceTimeStep )->TransferItkToVtkTransform();
  transform->SetMatrix(surfaceTimeGeometry->GetGeometry3D( surfaceTimeStep )->GetVtkTransform()->GetMatrix());
  transform->PostMultiply();
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
  surfaceConverter->DebugOn();
  surfaceConverter->SetTolerance( 0.0 );
  surfaceConverter->ReleaseDataFlagOn();

  surfaceConverter->SetInput( normalsFilter->GetOutput() );
  normalsFilter->Delete();

  vtkImageData * tmp = ( (mitk::Image*)GetImage() )->GetVtkImageData( time );

  vtkImageCast * castFilter = vtkImageCast::New();
  castFilter->ReleaseDataFlagOn();
  castFilter->SetOutputScalarTypeToInt();
  castFilter->SetInput( tmp );

  vtkImageStencil * stencil = vtkImageStencil::New();
  stencil->SetBackgroundValue( m_BackgroundValue );
  stencil->ReverseStencilOff();
  stencil->ReleaseDataFlagOn();

  stencil->SetStencil( surfaceConverter->GetOutput() );
  surfaceConverter->Delete();

  stencil->SetInput( castFilter->GetOutput() );
  castFilter->Delete();


  if (m_MakeOutputBinaryOn)
  {
    vtkImageThreshold * threshold = vtkImageThreshold::New();
    threshold->SetInput( stencil->GetOutput() );
    stencil->Delete();
    threshold->ThresholdByUpper(1);
    threshold->ReplaceInOn();
    threshold->ReplaceOutOn();
    threshold->SetInValue(1);
    threshold->SetOutValue(0);
//    threshold->SetOutputScalarTypeToUnsignedChar();
    threshold->Update();

    mitk::Image::Pointer output = this->GetOutput();
    output->SetVolume( threshold->GetOutput()->GetScalarPointer(), time );

    threshold->Delete();
  }
  else
  {
    stencil->Update();
    mitk::Image::Pointer output = this->GetOutput();
    output->SetVolume( stencil->GetOutput()->GetScalarPointer(), time );
    std::cout << "stencil ref count: " << stencil->GetReferenceCount() << std::endl;
    stencil->Delete();
  }
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
