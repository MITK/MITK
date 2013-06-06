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


  /*

  m_textActor = vtkSmartPointer<vtkTextActor>::New();
  m_textActor->GetTextProperty()->SetFontSize ( 24 );
  m_textActor->SetPosition2 ( 10, 40 );
  m_textActor->SetInput ( "Hello world" );
  m_textActor->GetTextProperty()->SetColor ( 1.0,0.0,0.0 );

  */
}


//void mitk::LabelOverlay3D::MitkRender(mitk::BaseRenderer *renderer)
//{
//  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
//  ls->m_follower->SetPosition(getPosition3D()[0],getPosition3D()[1],getPosition3D()[2]);
//  ls->m_textSource->SetText(getText().c_str());

////  renderer->GetRenderWindow()->GetInteractor();
//  if(!renderer->GetVtkRenderer()->HasViewProp(ls->m_follower))
//  {
//    ls->m_follower->SetCamera(renderer->GetVtkRenderer()->GetActiveCamera());
//    renderer->GetVtkRenderer()->AddActor2D(ls->m_follower);
//  }
//}


void mitk::LabelOverlay3D::setPosition3D(mitk::Point3D position3D)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("Pos3D", position3dProperty);
}

void mitk::LabelOverlay3D::setText(std::string text)
{
  SetStringProperty("text", text.c_str());
}

void mitk::LabelOverlay3D::setPosition3D(Point3D position3D, mitk::BaseRenderer *renderer)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("Pos3D", position3dProperty,renderer);
}

mitk::Point3D mitk::LabelOverlay3D::getPosition3D()
{
  mitk::Point3D position3D;
  GetPropertyValue < mitk::Point3D > ("Pos3D", position3D);
  return position3D;
}

std::string mitk::LabelOverlay3D::getText()
{
  std::string text;
  GetPropertyList()->GetStringProperty("text", text);
  return text;
}

mitk::Point3D mitk::LabelOverlay3D::getPosition3D(mitk::BaseRenderer *renderer)
{
  mitk::Point3D position3D;
  GetPropertyValue<mitk::Point3D>("Pos3D", position3D, renderer);
  return position3D;
}


vtkSmartPointer<vtkActor> mitk::LabelOverlay3D::getVtkActor(BaseRenderer *renderer)
{
}
