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
  m_BackgroundValue = -1;
}

mitk::SurfaceToImageFilter::~SurfaceToImageFilter()
{
}

void mitk::SurfaceToImageFilter::GenerateInputRequestedRegion()
{
  mitk::Image* output = this->GetOutput();
  if((output->IsInitialized()==false) )
    return;
  // we have already calculated the spatial part of the 
  // input-requested-region in m_InputRequestedRegion in 
  // GenerateOutputInformation (which is called before 
  // GenerateInputRequestedRegion).
  GenerateTimeInInputRegion(output, const_cast< mitk::Image * > ( this->GetImage() ));
//  GenerateTimeInInputRegion(output, this->GetImage()->GetLargestPossibleRegion() );
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
  slicedGeometry->GetVtkTransform()->Translate(start[0], start[1], start[2]);
  slicedGeometry->TransferVtkToITKTransform();

  mitk::TimeSlicedGeometry* timeSlicedGeometry = output->GetTimeSlicedGeometry();
  timeSlicedGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
  timeSlicedGeometry->CopyTimes(inputImage->GetTimeSlicedGeometry());

  //  m_TimeOfHeaderInitialization.Modified();
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
  //const mitk::TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();
  //const mitk::TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  //const mitk::TimeSlicedGeometry *boundingObjectTimeGeometry = m_BoundingObject->GetTimeSlicedGeometry();
  //ScalarType timeInMS;

  int timestep=0;
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
  
  int surfaceTimeStep = surfaceTimeGeometry->TimeStepToMS( imageTimeGeometry->TimeStepToMS(time) );
  
  vtkPolyData * polydata = ( (mitk::Surface*)GetInput() )->GetVtkPolyData( surfaceTimeStep );
  vtkTransformPolyDataFilter * move=vtkTransformPolyDataFilter::New();
  move->SetInput(polydata);
  vtkTransform *transform=vtkTransform::New();
  surfaceTimeGeometry->GetGeometry3D( surfaceTimeStep )->TransferItkToVtkTransform();
  transform->SetMatrix(surfaceTimeGeometry->GetGeometry3D( surfaceTimeStep )->GetVtkTransform()->GetMatrix());
//  transform->Inverse();
  transform->PostMultiply();
  //  transform->Translate(0.5,0.5,0.5); /// warum????
  move->SetTransform(transform);
  move->Update();
  polydata=move->GetOutput();

  vtkPolyDataNormals * normalsFilter = vtkPolyDataNormals::New();
  normalsFilter->SetInput( polydata );
  normalsFilter->SetFeatureAngle(50);
  normalsFilter->SetConsistency(1);
  normalsFilter->SetSplitting(1);
  normalsFilter->SetFlipNormals(0);
  normalsFilter->Update();

  vtkPolyDataToImageStencil * surfaceConverter = vtkPolyDataToImageStencil::New();
  surfaceConverter->SetInput( normalsFilter->GetOutput() );
  surfaceConverter->SetTolerance( 0.0 );
  surfaceConverter->Update();

  vtkImageData * tmp = ( (mitk::Image*)GetImage() )->GetVtkImageData( time );
  vtkImageMathematics * maths = vtkImageMathematics::New();
  maths->SetOperationToAddConstant();
  maths->SetConstantC(0);
  maths->SetConstantK(100);
  maths->SetInput1(tmp);
  maths->Update();

  vtkImageCast * castFilter = vtkImageCast::New();
  castFilter->SetOutputScalarTypeToInt();
  castFilter->SetInput( tmp );

  vtkImageStencil * stencil = vtkImageStencil::New();
  stencil->SetStencil( surfaceConverter->GetOutput() );
  stencil->SetBackgroundValue( m_BackgroundValue );
  stencil->ReverseStencilOff();
  stencil->SetInput( castFilter->GetOutput() );
  stencil->Update();

  if (m_MakeOutputBinaryOn)
  {
    vtkImageThreshold * threshold = vtkImageThreshold::New();
    threshold->SetInput( stencil->GetOutput() );
    threshold->ThresholdByUpper(1);
    threshold->ReplaceInOn();
    threshold->ReplaceOutOn();
    threshold->SetInValue(1);
    threshold->SetOutValue(0);
    threshold->Update();

    vtkImageCast * castFilter = vtkImageCast::New();
    castFilter->SetOutputScalarTypeToInt();
    castFilter->SetInput(threshold->GetOutput() );
    castFilter->Update();

    mitk::Image::Pointer output = this->GetOutput();
    //output->Initialize( castFilter->GetOutput() );
    //output->SetGeometry( static_cast<mitk::Geometry3D*>(GetImage()->GetGeometry()->Clone().GetPointer()) );
    output->SetVolume( stencil->GetOutput()->GetScalarPointer(), time );
  }
  else
  {
    mitk::Image::Pointer output = this->GetOutput();
    output->SetVolume( stencil->GetOutput()->GetScalarPointer(), time );
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
