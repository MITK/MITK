/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 12006 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkParametricCurveVtkMapper3D.h"
#include "mitkParametricCurve.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"

#include <vtkTubeFilter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>

#include <vtkTransform.h>

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProp3D.h>
#include <vtkPointData.h>

#include <vtkAxes.h>

#include <stdlib.h>

namespace mitk
{

ParametricCurveVtkMapper3D
::ParametricCurveVtkMapper3D() 
{
  m_Actor = vtkActor::New();
  m_Prop3D = m_Actor;

  m_Mapper = vtkPolyDataMapper::New();

  m_TubeFilter = vtkTubeFilter::New();
  m_CurveData = vtkPolyData::New();
  m_CurvePoints = vtkPoints::New();
  m_CurveLines = vtkCellArray::New();
}


ParametricCurveVtkMapper3D
::~ParametricCurveVtkMapper3D()
{
  m_Actor->Delete();
  m_Mapper->Delete();
  m_TubeFilter->Delete();
  m_CurveData->Delete();
  m_CurvePoints->Delete();
  m_CurveLines->Delete();
}


const ParametricCurve *
ParametricCurveVtkMapper3D
::GetInput()
{
  return static_cast< const ParametricCurve * > ( this->GetData() );
}


void 
ParametricCurveVtkMapper3D
::GenerateData()
{
  mitk::ParametricCurve::Pointer curve  = 
    const_cast< mitk::ParametricCurve * >( this->GetInput() );

  int timeStep = 0;

  if ( !curve->IsValidCurve( timeStep ) )
  {
    // Do not display anything: remove mapper
    m_Actor->SetMapper( NULL );
    return;
  }

  ScalarType rangeMin = curve->GetRangeMin( timeStep );
  ScalarType rangeMax = curve->GetRangeMax( timeStep );
  
  m_CurvePoints->Initialize();
  m_CurvePoints->SetNumberOfPoints( 400 );
  int i;
  for ( i = 0; i < 400; ++i )
  {
    Point3D point = curve->Evaluate( rangeMin + ((rangeMax - rangeMin) / 399.0) * i, timeStep );
    m_CurvePoints->InsertPoint( i, point[0], point[1], point[2] );
    std::cout << "P: " << point[0] << ", " << point[1] << ", " << point[2] << std::endl;
  }

  m_CurveLines->Initialize();
  m_CurveLines->InsertNextCell( 400 );
  for ( i = 0; i < 400; ++i )
  {
    m_CurveLines->InsertCellPoint( i );
  }

  m_CurveData->SetPoints( m_CurvePoints );
  m_CurveData->SetLines( m_CurveLines );

  vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
  colors->SetName( "colors" );
  colors->SetNumberOfComponents( 3 );
  for ( i = 0; i < 400; ++i )
  {
    colors->InsertTuple3( i, i/3 + 120, i/2.5 + 50, i/2 + 30 );
  }

  m_CurveData->GetPointData()->SetScalars( colors );

  m_TubeFilter->SetInput( m_CurveData );
  m_TubeFilter->SetNumberOfSides( 8 );
  m_TubeFilter->SetRadius( 2 );

  m_Mapper->SetInput( m_TubeFilter->GetOutput() );
  m_Actor->SetMapper( m_Mapper );
}

void 
ParametricCurveVtkMapper3D
::GenerateData( BaseRenderer *renderer )
{
}

} // namespace mitk
