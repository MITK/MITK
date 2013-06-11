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

#include "mitkLabelOverlay3D.h"
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>


mitk::LabelOverlay3D::LabelOverlay3D()
{
}


mitk::LabelOverlay3D::~LabelOverlay3D()
{
}

mitk::LabelOverlay3D::LocalStorage::~LocalStorage()
{
}

mitk::LabelOverlay3D::LocalStorage::LocalStorage()
{
  // Create some text
  m_textSource = vtkSmartPointer<vtkVectorText>::New();
  m_textSource->SetText( "Hello" );

  // Create a mapper
  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( m_textSource->GetOutputPort() );

  // Create a subclass of vtkActor: a vtkFollower that remains facing the camera
  m_follower = vtkSmartPointer<vtkFollower>::New();
  m_follower->SetMapper( mapper );
  m_follower->GetProperty()->SetColor( 1, 0, 0 ); // red
  m_follower->SetScale(1);
}

void mitk::LabelOverlay3D::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  ls->m_follower->SetPosition(GetPosition3D()[0],GetPosition3D()[1],GetPosition3D()[2]);
  ls->m_textSource->SetText(GetText().c_str());
  float color[3] = {1,1,1};
  float opacity = 1.0;
  GetColor(color,renderer);
  GetOpacity(opacity,renderer);
  ls->m_follower->GetProperty()->SetColor(color[0], color[1], color[2]);
  ls->m_follower->GetProperty()->SetOpacity(opacity);
}



void mitk::LabelOverlay3D::SetPosition3D(mitk::Point3D position3D)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("Pos3D", position3dProperty);
}

void mitk::LabelOverlay3D::SetText(std::string text)
{
  SetStringProperty("text", text.c_str());
}

void mitk::LabelOverlay3D::SetPosition3D(Point3D position3D, mitk::BaseRenderer *renderer)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("Pos3D", position3dProperty,renderer);
}

mitk::Point3D mitk::LabelOverlay3D::GetPosition3D()
{
  mitk::Point3D position3D;
  GetPropertyValue < mitk::Point3D > ("Pos3D", position3D);
  return position3D;
}

std::string mitk::LabelOverlay3D::GetText()
{
  std::string text;
  GetPropertyList()->GetStringProperty("text", text);
  return text;
}

mitk::Point3D mitk::LabelOverlay3D::GetPosition3D(mitk::BaseRenderer *renderer)
{
  mitk::Point3D position3D;
  GetPropertyValue<mitk::Point3D>("Pos3D", position3D, renderer);
  return position3D;
}


vtkSmartPointer<vtkProp> mitk::LabelOverlay3D::GetVtkProp(BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  ls->m_follower->SetCamera(renderer->GetVtkRenderer()->GetActiveCamera());
  return ls->m_follower;
}
