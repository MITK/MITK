/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSplineVtkMapper3D.h"
#include <vtkProp.h>
#include <vtkAssembly.h>
#include <vtkCardinalSpline.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>
#include <mitkProperties.h>

mitk::SplineVtkMapper3D::SplineVtkMapper3D()
        : m_SplinesActor( NULL ), m_Assembly( NULL )
{
    m_SplinesActor = vtkActor::New();
    m_Assembly = vtkAssembly::New();
    m_SplinesAddedToAssembly = false;
    m_SplinesAvailable = false;

    m_Assembly->AddPart( m_Prop3D );
}




mitk::SplineVtkMapper3D::~SplineVtkMapper3D()
{
    if ( m_SplinesActor != NULL )
        m_SplinesActor->Delete();

    if ( m_Assembly != NULL )
        m_Assembly->Delete();
}




vtkProp*
mitk::SplineVtkMapper3D::GetProp()
{
    if ( GetDataTreeNode() != NULL && m_Assembly != NULL )
    {
        m_Assembly->SetUserTransform( GetDataTreeNode()->GetVtkTransform() );
    }

    return m_Assembly;
}


void
mitk::SplineVtkMapper3D::GenerateData()
{
    mitk::PointSet::Pointer input = const_cast<mitk::PointSet*>( this->GetInput( ) );
    input->Update();

    // Number of points on the spline
    unsigned int numberOfOutputPoints = 400;
    unsigned int numberOfInputPoints = input->GetSize();


    if ( numberOfInputPoints >= 2 )
    {
        m_SplinesAvailable = true;
        vtkCardinalSpline* splineX = vtkCardinalSpline::New();
        vtkCardinalSpline* splineY = vtkCardinalSpline::New();
        vtkCardinalSpline* splineZ = vtkCardinalSpline::New();

        for ( unsigned int i = 0 ; i < numberOfInputPoints; ++i )
        {
            mitk::PointSet::PointType point = input->GetPoint( i );
            splineX->AddPoint( i, point[ 0 ] );
            splineY->AddPoint( i, point[ 1 ] );
            splineZ->AddPoint( i, point[ 2 ] );
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
        profileMapper->SetInput( profileData );

        m_SplinesActor->SetMapper( profileMapper );
        float rgba[ 4 ] = {1.0f, 0.0f, 0.0f, 1.0f};
        this->GetDataTreeNode()->GetColor( rgba, NULL );
        m_SplinesActor->GetProperty()->SetColor( rgba );
    }
    else
    {
        m_SplinesAvailable = false;
    }


    if ( m_SplinesAvailable )
    {
        if ( ! m_SplinesAddedToAssembly )
        {
            m_Assembly->AddPart( m_SplinesActor );
            m_SplinesAddedToAssembly = true;
        }
    }
    else
    {
        if ( m_SplinesAddedToAssembly )
        {
            m_Assembly->RemovePart( m_SplinesActor );
            m_SplinesAddedToAssembly = false;
        }
    }
}


void mitk::SplineVtkMapper3D::GenerateData( mitk::BaseRenderer* renderer )
{
  bool doNotDrawPoints;
  if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("dontdrawpoints").GetPointer()) == NULL)
    doNotDrawPoints = false;
  else
    doNotDrawPoints = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("dontdrawpoints").GetPointer())->GetValue();

  Superclass::GenerateData( renderer );

  if ( IsVisible( renderer ) == false )
  {
    if ( m_SplinesActor != NULL )
      m_SplinesActor->VisibilityOff();
    if ( m_Prop3D && doNotDrawPoints )
      m_Prop3D->VisibilityOff();
    if ( m_Assembly != NULL )
      m_Assembly->VisibilityOff();
  }
  else
  {
    if ( m_SplinesActor != NULL )
      m_SplinesActor->VisibilityOn();
    if ( m_Prop3D )
    {
      if ( doNotDrawPoints )
        m_Prop3D->VisibilityOff();
      else
        m_Prop3D->VisibilityOn();
    }

    if ( m_Assembly != NULL )
      m_Assembly->VisibilityOn();
  }
}

bool mitk::SplineVtkMapper3D::SplinesAreAvailable()
{
    return m_SplinesAvailable;
}


vtkPolyData* mitk::SplineVtkMapper3D::GetSplinesPolyData()
{
    Mapper::Update(NULL);
    if ( m_SplinesAvailable )
        return ( dynamic_cast<vtkPolyDataMapper*>( m_SplinesActor->GetMapper() ) )->GetInput();
    else
        return vtkPolyData::New();
}

vtkActor* mitk::SplineVtkMapper3D::GetSplinesActor()
{
    Mapper::Update(NULL);
    if ( m_SplinesAvailable )
        return m_SplinesActor;
    else
        return vtkActor::New();
}


vtkPolyData* mitk::SplineVtkMapper3D::GetPointsPolyData()
{
    Mapper::Update(NULL);
    if (m_vtkPointList->GetOutput())
        return m_vtkPointList->GetOutput();
    else
        return vtkPolyData::New();
}

vtkActor* mitk::SplineVtkMapper3D::GetPointsActor()
{
    Mapper::Update(NULL);
    if (m_Actor)
        return m_Actor;
    else
        return vtkActor::New();
}


