/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkProbeFilter.h"
#include "mitkSurface.h"
#include "mitkImage.h"
#include "mitkTimeSlicedGeometry.h"

#include <vtkPolyDataSource.h>
#include <vtkProbeFilter.h>
#include <itkImageRegion.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

#include <itkSmartPointerForwardReference.txx>


mitk::ProbeFilter::ProbeFilter()
{

}

mitk::ProbeFilter::~ProbeFilter()
{

}

const mitk::Surface *mitk::ProbeFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }
  return static_cast< const mitk::Surface * >(this->ProcessObject::GetInput(0) );
}

const mitk::Image *mitk::ProbeFilter::GetSource(void)
{
  return static_cast< const mitk::Image * >(this->ProcessObject::GetInput(1));		
}

void mitk::ProbeFilter::SetInput(const mitk::Surface *input)
{
  this->ProcessObject::SetNthInput( 0, const_cast< mitk::Surface * >( input ) );
}

void mitk::ProbeFilter::SetSource(const mitk::Image *source)
{
  this->ProcessObject::SetNthInput( 1, const_cast< mitk::Image * >( source ) );	
}

void mitk::ProbeFilter::GenerateOutputInformation()
{
  mitk::Surface::ConstPointer input  = this->GetInput();	
  mitk::Image::ConstPointer source = this->GetSource();	
  mitk::Surface::Pointer output = this->GetOutput();

  if(input.IsNull()) return;
  if(source.IsNull()) return;

  if(input->GetGeometry()==NULL) return;  
  if(source->GetGeometry()==NULL) return; 

  if( (input->GetTimeSlicedGeometry()->GetTimeSteps()==1) && (source->GetTimeSlicedGeometry()->GetTimeSteps()>1) )
  {
    Geometry3D::Pointer geometry3D = Geometry3D::New();
    geometry3D->Initialize();
    geometry3D->SetBounds(source->GetTimeSlicedGeometry()->GetBounds());
    geometry3D->SetTimeBounds(source->GetTimeSlicedGeometry()->GetGeometry3D(0)->GetTimeBounds());

    TimeSlicedGeometry::Pointer outputTimeSlicedGeometry = TimeSlicedGeometry::New();
    outputTimeSlicedGeometry->InitializeEvenlyTimed(geometry3D, source->GetTimeSlicedGeometry()->GetTimeSteps());

    output->Resize(outputTimeSlicedGeometry->GetTimeSteps());
    output->SetGeometry( outputTimeSlicedGeometry );
  }
  else
    output->SetGeometry( static_cast<Geometry3D*>(input->GetGeometry()->Clone().GetPointer()) );

  itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::ProbeFilter::GenerateData()
{
  mitk::Surface *input  = const_cast< mitk::Surface * >(this->GetInput());	
  mitk::Image *source = const_cast< mitk::Image * >(this->GetSource());
  mitk::Surface::Pointer output = this->GetOutput();

  itkDebugMacro(<<"Generating Data");

  if(output.IsNull())
  {
    itkDebugMacro(<<"Output is NULL.");
    return;
  }

  mitk::Surface::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *sourceTimeGeometry = source->GetTimeSlicedGeometry();
  ScalarType timeInMS;
  int timestep=0;

  int tstart, tmax;

  tstart=outputRegion.GetIndex(3);
  tmax=tstart+outputRegion.GetSize(3);

  int t;
  for(t=tstart;t<tmax;++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToMS( t );

    vtkProbeFilter* probe = vtkProbeFilter::New();

    timestep = inputTimeGeometry->MSToTimeStep( timeInMS );
    probe->SetInput( input->GetVtkPolyData(timestep) );

    timestep = sourceTimeGeometry->MSToTimeStep( timeInMS );
    probe->SetSource( source->GetVtkImageData(timestep) );

    output->SetVtkPolyData( probe->GetPolyDataOutput(), t );  

    probe->Update();
    probe->Delete();
  }
}

void mitk::ProbeFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Surface *input  = const_cast< mitk::Surface * >( this->GetInput() );	
  mitk::Image *source = const_cast< mitk::Image * >( this->GetSource() );

  if(input==NULL) return;
  if(source==NULL) return;

  mitk::Surface::Pointer output = this->GetOutput();
  mitk::Surface::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();

  mitk::Surface::RegionType inputSurfaceRegion = outputRegion;
  Image::RegionType sourceImageRegion = source->GetLargestPossibleRegion();

  if(outputRegion.GetSize(3)<1)
  {
    mitk::Surface::RegionType::SizeType surfacesize;
    surfacesize.Fill(0);
    inputSurfaceRegion.SetSize(surfacesize);
    input->SetRequestedRegion( &inputSurfaceRegion  );
    mitk::Image::RegionType::SizeType imagesize;
    imagesize.Fill(0);
    sourceImageRegion.SetSize(imagesize);
    source->SetRequestedRegion( &sourceImageRegion );
    return;
  }

  //create and set input requested region for the input surface
  const mitk::TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();

  ScalarType timeInMS;
  int timestep=0;

  // convert the start-index-time of output in start-index-time of input via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToMS(outputRegion.GetIndex(3));
  timestep = inputTimeGeometry->MSToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( inputTimeGeometry->IsValidTime( timestep ) ) )
    inputSurfaceRegion.SetIndex( 3, timestep );
  else
    inputSurfaceRegion.SetIndex( 3, 0 );
  // convert the end-index-time of output in end-index-time of input via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToMS(outputRegion.GetIndex(3)+outputRegion.GetSize(3)-1);
  timestep = inputTimeGeometry->MSToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( outputTimeGeometry->IsValidTime( timestep ) ) )
    inputSurfaceRegion.SetSize( 3, timestep - inputSurfaceRegion.GetIndex(3) + 1 );
  else
    inputSurfaceRegion.SetSize( 3, 1 );

  input->SetRequestedRegion( &inputSurfaceRegion  );

  //create and set input requested region for the source image
  const mitk::TimeSlicedGeometry *sourceTimeGeometry = source->GetTimeSlicedGeometry();

  // convert the start-index-time of output in start-index-time of source via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToMS(outputRegion.GetIndex(3));
  timestep = sourceTimeGeometry->MSToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( sourceTimeGeometry->IsValidTime( timestep ) ) )
    sourceImageRegion.SetIndex( 3, timestep );
  else
    sourceImageRegion.SetIndex( 3, 0 );
  // convert the end-index-time of output in end-index-time of source via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToMS(outputRegion.GetIndex(3)+outputRegion.GetSize(3)-1);
  timestep = sourceTimeGeometry->MSToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( outputTimeGeometry->IsValidTime( timestep ) ) )
    sourceImageRegion.SetSize( 3, timestep - sourceImageRegion.GetIndex(3) + 1 );
  else
    sourceImageRegion.SetSize( 3, 1 );

  sourceImageRegion.SetIndex( 4, 0 );
  sourceImageRegion.SetSize( 4, 1 );

  source->SetRequestedRegion( &sourceImageRegion );
}
