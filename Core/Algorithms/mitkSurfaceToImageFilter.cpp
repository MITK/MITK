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

#include <vtkPolyData.h>

#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkImageThreshold.h>
#include <vtkImageMathematics.h>
#include <vtkPolyDataNormals.h>

#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>

mitk::SurfaceToImageFilter::SurfaceToImageFilter()
{

}

mitk::SurfaceToImageFilter::~SurfaceToImageFilter()
{
}

void mitk::SurfaceToImageFilter::GenerateOutputInformation()
{
  mitk::Surface::ConstPointer input  = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull()) return;

//  output->SetGeometry(static_cast<Geometry3D*>(input->GetGeometry()->Clone().GetPointer()));
}

void mitk::SurfaceToImageFilter::GenerateData()
{
  vtkPolyData * polydata = ( (mitk::Surface*)GetInput() )->GetVtkPolyData();

  vtkTransformPolyDataFilter * move=vtkTransformPolyDataFilter::New();
  move->SetInput(polydata);
  vtkTransform *transform=vtkTransform::New();
  mitk::Vector3D spacing=GetImage()->GetSlicedGeometry()->GetSpacing();
  spacing*=0.5;
  transform->Translate(spacing[0],spacing[1],spacing[2]);
  move->SetTransform(transform);

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

  vtkImageData * tmp = ( (mitk::Image*)GetImage() )->GetVtkImageData();
  vtkImageMathematics * maths = vtkImageMathematics::New();
  maths->SetOperationToAddConstant();
  maths->SetConstantC(100);
  maths->SetConstantK(100);
  maths->SetInput1(tmp);
  maths->Update();

  vtkImageStencil * stencil = vtkImageStencil::New();
  stencil->SetStencil( surfaceConverter->GetOutput() );
  stencil->SetBackgroundValue( 0 );
  stencil->ReverseStencilOff();
  stencil->SetInput( maths->GetOutput() );
  stencil->Update();

  vtkImageThreshold * threshold = vtkImageThreshold::New();
  threshold->SetInput( stencil->GetOutput() );
  threshold->ThresholdByUpper(1);
  threshold->ReplaceInOn();
  threshold->SetInValue(1);
  threshold->Update();

  mitk::Image::Pointer output = this->GetOutput();
  output->Initialize( threshold->GetOutput() );
  output->SetVolume( threshold->GetOutput()->GetScalarPointer() );

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
