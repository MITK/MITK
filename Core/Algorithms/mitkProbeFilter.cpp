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

////coming from mitkGeometry2DDataToSurfaceFilter.cpp
=========================================================================*/

#include "mitkProbeFilter.h"
#include "mitkSurface.h"
#include "mitkTimeSlicedGeometry.h"

#include <vtkPolyDataSource.h>
#include <vtkProbeFilter.h>
#include <itkImageRegion.h>

mitk::ProbeFilter::ProbeFilter()
//  : m_UseGeometryParametricBounds(true), m_XResolution(10), m_YResolution(10)
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

/// input volume of ultrasound doppler data
void mitk::ProbeFilter::SetSource(const mitk::Image *source)
{
  this->ProcessObject::SetNthInput( 1, const_cast< mitk::Image * >( source ) );	
}

void mitk::ProbeFilter::GenerateOutputInformation()
{
  unsigned int idx = 0;
  mitk::Surface::ConstPointer input  = this->GetInput();	
  mitk::Image::ConstPointer source = this->GetSource();	
  mitk::Surface::Pointer output = this->GetOutput();

  if(input.IsNull()) return;
  if(source.IsNull()) return;

  if(input->GetGeometry()==NULL) return;  
  if(source->GetGeometry()==NULL) return; 

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

  //	for( int t=outputRegion[0]; t<outputRegion[1]; ++t )
  for( int t=outputRegion.GetIndex(3); t<(outputRegion.GetIndex(3)+outputRegion.GetSize(3)); ++t )
  {
    vtkProbeFilter* probe = vtkProbeFilter::New();
    probe->SetInput( input->GetVtkPolyData(t) );
    probe->SetSource( source->GetVtkImageData(t) );

    output->SetVtkPolyData( probe->GetPolyDataOutput(), t );  
    probe->Update();
    probe->Delete();
  }
}

void mitk::ProbeFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Surface::Pointer output = this->GetOutput();
  //mitk::Surface::RegionType outputRegion = const_cast<mitk::Surface::RegionType& >(output->GetTimeSlicedGeometry()->GetRequestedRegion());
  mitk::Surface::RegionType outputRegion = output->GetRequestedRegion();
  mitk::Geometry3D *outputRegionGeometry = const_cast< mitk::TimeSlicedGeometry * >(output->GetTimeSlicedGeometry());

  //create and set input requested region for the input surface
  mitk::Surface::RegionType inputSurfaceRegion = outputRegion;
  mitk::Surface *input  = const_cast< mitk::Surface * >( this->GetInput() );	

  input->SetRequestedRegion( &inputSurfaceRegion  );
  //input->SetGeometry( outputRegionGeometry );

  //create and set input requested region for the source image
  Image::RegionType slicedDataRegion;
  mitk::Image *source = const_cast< mitk::Image * >( this->GetSource() );

  slicedDataRegion = source->GetLargestPossibleRegion();
  slicedDataRegion.SetIndex( 3, outputRegion.GetIndex(3) );
  slicedDataRegion.SetSize( 3, outputRegion.GetSize(3) );   

  source->SetRequestedRegion( &slicedDataRegion );
}

