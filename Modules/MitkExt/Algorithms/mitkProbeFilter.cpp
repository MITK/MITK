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

#include "mitkProbeFilter.h"
#include "mitkSurface.h"
#include "mitkImage.h"

#include <vtkProbeFilter.h>
#include <itkImageRegion.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

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

  if( (input->GetTimeGeometry()->CountTimeSteps()==1) && (source->GetTimeGeometry()->CountTimeSteps()>1) )
  {
    BaseGeometry::Pointer geometry3D = BaseGeometry::New();
    geometry3D->Initialize();
    geometry3D->SetBounds(source->GetTimeGeometry()->GetBoundsInWorld());
    geometry3D->SetTimeBounds(source->GetTimeGeometry()->GetGeometryForTimeStep(0)->GetTimeBounds());

    ProportionalTimeGeometry::Pointer outputTimeGeometry = ProportionalTimeGeometry::New();
    outputTimeGeometry->Initialize(geometry3D, source->GetTimeGeometry()->CountTimeSteps());

    output->Expand(outputTimeGeometry->CountTimeSteps());
    output->SetTimeGeometry( outputTimeGeometry );
  }
  else
    output->SetGeometry( static_cast<BaseGeometry*>(input->GetGeometry()->Clone().GetPointer()) );

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
  const TimeGeometry *outputTimeGeometry = output->GetTimeGeometry();
  const TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
  const TimeGeometry *sourceTimeGeometry = source->GetTimeGeometry();
  TimePointType timeInMS;
  int timestep=0;

  int tstart, tmax;

  tstart=outputRegion.GetIndex(3);
  tmax=tstart+outputRegion.GetSize(3);

  int t;
  for(t=tstart;t<tmax;++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToTimePoint( t );

    vtkProbeFilter* probe = vtkProbeFilter::New();

    timestep = inputTimeGeometry->TimePointToTimeStep( timeInMS );
    probe->SetInputData( input->GetVtkPolyData(timestep) );

    timestep = sourceTimeGeometry->TimePointToTimeStep( timeInMS );
    probe->SetSourceData( source->GetVtkImageData(timestep) );

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
  const TimeGeometry *outputTimeGeometry = output->GetTimeGeometry();

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
  const TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();

  ScalarType timeInMS;
  int timestep=0;

  // convert the start-index-time of output in start-index-time of input via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToTimePoint(outputRegion.GetIndex(3));
  timestep = inputTimeGeometry->TimePointToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( inputTimeGeometry->IsValidTimeStep( timestep ) ) )
    inputSurfaceRegion.SetIndex( 3, timestep );
  else
    inputSurfaceRegion.SetIndex( 3, 0 );
  // convert the end-index-time of output in end-index-time of input via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToTimePoint(outputRegion.GetIndex(3)+outputRegion.GetSize(3)-1);
  timestep = inputTimeGeometry->TimePointToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( outputTimeGeometry->IsValidTimeStep( timestep ) ) )
    inputSurfaceRegion.SetSize( 3, timestep - inputSurfaceRegion.GetIndex(3) + 1 );
  else
    inputSurfaceRegion.SetSize( 3, 1 );

  input->SetRequestedRegion( &inputSurfaceRegion  );

  //create and set input requested region for the source image
  const TimeGeometry *sourceTimeGeometry = source->GetTimeGeometry();

  // convert the start-index-time of output in start-index-time of source via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToTimePoint(outputRegion.GetIndex(3));
  timestep = sourceTimeGeometry->TimePointToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( sourceTimeGeometry->IsValidTimeStep( timestep ) ) )
    sourceImageRegion.SetIndex( 3, timestep );
  else
    sourceImageRegion.SetIndex( 3, 0 );
  // convert the end-index-time of output in end-index-time of source via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToTimePoint(outputRegion.GetIndex(3)+outputRegion.GetSize(3)-1);
  timestep = sourceTimeGeometry->TimePointToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( outputTimeGeometry->IsValidTimeStep( timestep ) ) )
    sourceImageRegion.SetSize( 3, timestep - sourceImageRegion.GetIndex(3) + 1 );
  else
    sourceImageRegion.SetSize( 3, 1 );

  sourceImageRegion.SetIndex( 4, 0 );
  sourceImageRegion.SetSize( 4, 1 );

  source->SetRequestedRegion( &sourceImageRegion );
}
