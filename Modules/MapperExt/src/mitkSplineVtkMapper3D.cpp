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


#include "mitkSplineVtkMapper3D.h"
#include <vtkProp.h>
#include <vtkPropAssembly.h>
#include <vtkCardinalSpline.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>
#include <vtkPropCollection.h>
#include <mitkProperties.h>
#include <mitkPointSet.h>


mitk::SplineVtkMapper3D::SplineVtkMapper3D()
: m_SplinesAvailable (false), m_SplinesAddedToAssembly(false)
{
  m_SplinesActor = vtkActor::New();
  m_SplineAssembly = vtkPropAssembly::New();
  m_SplineResolution = 500;
}


mitk::SplineVtkMapper3D::~SplineVtkMapper3D()
{
  m_SplinesActor->Delete();
  m_SplineAssembly->Delete();
}

vtkProp*
mitk::SplineVtkMapper3D::GetVtkProp(mitk::BaseRenderer * /*renderer*/)
{
  return m_SplineAssembly;
}

void mitk::SplineVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer * /*renderer*/)
{
  vtkLinearTransform * vtktransform =
    this->GetDataNode()->GetVtkTransform(this->GetTimestep());

  m_SplinesActor->SetUserTransform(vtktransform);
}


void mitk::SplineVtkMapper3D::GenerateDataForRenderer( mitk::BaseRenderer* renderer )
{
  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  // only update spline if UpdateSpline has not been called from
  // external, e.g. by the SplineMapper2D. But call it the first time when m_SplineUpdateTime = 0 and m_LastUpdateTime = 0.
  if ( m_SplineUpdateTime < ls->GetLastGenerateDataTime() || m_SplineUpdateTime == 0)
  {
    this->UpdateSpline();
    this->ApplyAllProperties(renderer, m_SplinesActor);
  }

  if ( m_SplinesAvailable )
  {
    if ( ! m_SplinesAddedToAssembly )
    {
      m_SplineAssembly->AddPart( m_SplinesActor );
      m_SplinesAddedToAssembly = true;
    }
  }
  else
  {
    if ( m_SplinesAddedToAssembly )
    {
      m_SplineAssembly->RemovePart( m_SplinesActor );
      m_SplinesAddedToAssembly = false;
    }
  }

   bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if(!visible)
  {
    m_SplinesActor->VisibilityOff();
    m_SplineAssembly->VisibilityOff();
  }
  else
  {
    m_SplinesActor->VisibilityOn();
    m_SplineAssembly->VisibilityOn();

    //remove the PointsAssembly if it was added in superclass. No need to display points and spline!
    if(m_SplineAssembly->GetParts()->IsItemPresent(m_PointsAssembly))
      m_SplineAssembly->RemovePart(m_PointsAssembly);
  }
  //if the properties have been changed, then refresh the properties
  if ( (m_SplineUpdateTime < this->m_DataNode->GetPropertyList()->GetMTime() ) ||
       (m_SplineUpdateTime < this->m_DataNode->GetPropertyList(renderer)->GetMTime() ) )
    this->ApplyAllProperties(renderer, m_SplinesActor);
}


void mitk::SplineVtkMapper3D::ApplyAllProperties(BaseRenderer *renderer, vtkActor *actor)
{
  Superclass::ApplyColorAndOpacityProperties(renderer, actor);

  //vtk changed the type of rgba during releases. Due to that, the following convert is done
  double rgba[ 4 ] = {1.0f, 1.0f, 1.0f, 1.0f};//white

  //getting the color from DataNode
  float temprgba[4];
  this->GetDataNode()->GetColor( &temprgba[0], NULL );
  //convert to rgba, what ever type it has!
  rgba[0] = temprgba[0];    rgba[1] = temprgba[1];    rgba[2] = temprgba[2];    rgba[3] = temprgba[3];
  //finaly set the color inside the actor
  m_SplinesActor->GetProperty()->SetColor( rgba );

  float lineWidth;
  if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("line width")) == NULL)
    lineWidth = 1.0;
  else
    lineWidth = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("line width"))->GetValue();
  m_SplinesActor->GetProperty()->SetLineWidth(lineWidth);

  m_SplineUpdateTime.Modified();
}


bool mitk::SplineVtkMapper3D::SplinesAreAvailable()
{
  return m_SplinesAvailable;
}


vtkPolyData* mitk::SplineVtkMapper3D::GetSplinesPolyData()
{
  if ( m_SplinesAvailable )
    return ( dynamic_cast<vtkPolyDataMapper*>( m_SplinesActor->GetMapper() ) )->GetInput();
  else
    return NULL;
}

vtkActor* mitk::SplineVtkMapper3D::GetSplinesActor()
{
  if ( m_SplinesAvailable )
    return m_SplinesActor;
  else
    return vtkActor::New();
}


void mitk::SplineVtkMapper3D::UpdateSpline()
{
  mitk::PointSet::Pointer input = const_cast<mitk::PointSet*>( this->GetInput( ) );
//  input->Update();//already done in superclass

  // Number of points on the spline
  unsigned int numberOfOutputPoints = m_SplineResolution;
  unsigned int numberOfInputPoints = input->GetSize();


  if ( numberOfInputPoints >= 2 )
  {
    m_SplinesAvailable = true;
    vtkCardinalSpline* splineX = vtkCardinalSpline::New();
    vtkCardinalSpline* splineY = vtkCardinalSpline::New();
    vtkCardinalSpline* splineZ = vtkCardinalSpline::New();
    unsigned int index = 0;
    mitk::PointSet::DataType::PointsContainer::Pointer pointsContainer = input->GetPointSet()->GetPoints();
    for ( mitk::PointSet::DataType::PointsContainer::Iterator it = pointsContainer->Begin(); it != pointsContainer->End(); ++it, ++index )
    //for ( unsigned int i = 0 ; i < numberOfInputPoints; ++i )
    {
      mitk::PointSet::PointType point = it->Value();
      splineX->AddPoint( index, point[ 0 ] );
      splineY->AddPoint( index, point[ 1 ] );
      splineZ->AddPoint( index, point[ 2 ] );
    }
    vtkPoints* points = vtkPoints::New();
    vtkPolyData* profileData = vtkPolyData::New();


    // Interpolate x, y and z by using the three spline filters and
    // create new points
    double t = 0.0f;
    for ( unsigned int i = 0; i < numberOfOutputPoints; ++i )
    {
      t = ( ( ( ( double ) numberOfInputPoints ) - 1.0f ) / ( ( ( double ) numberOfOutputPoints ) - 1.0f ) ) * ( ( double ) i );
      points->InsertPoint( i, splineX->Evaluate( t ), splineY->Evaluate( t ), splineZ->Evaluate( t ) ) ;
    }

    // Create the polyline.
    vtkCellArray* lines = vtkCellArray::New();
    lines->InsertNextCell( numberOfOutputPoints );
    for ( unsigned int i = 0; i < numberOfOutputPoints; ++i )
      lines->InsertCellPoint( i );

    profileData->SetPoints( points );
    profileData->SetLines( lines );

    // Add thickness to the resulting line.
    //vtkTubeFilter* profileTubes = vtkTubeFilter::New();
    //profileTubes->SetNumberOfSides(8);
    //profileTubes->SetInput(profileData);
    //profileTubes->SetRadius(.005);

    vtkPolyDataMapper* profileMapper = vtkPolyDataMapper::New();
    profileMapper->SetInputData( profileData );

    m_SplinesActor->SetMapper( profileMapper );
  }
  else
  {
    m_SplinesAvailable = false;
  }
  m_SplineUpdateTime.Modified();
}
