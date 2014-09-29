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

#include "mitkTextOverlay3D.h"
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkFollower.h>
#include <vtkVectorText.h>
#include <vtkTextActor3D.h>


mitk::TextOverlay3D::TextOverlay3D()
{
}


mitk::TextOverlay3D::~TextOverlay3D()
{
}

mitk::TextOverlay3D::LocalStorage::~LocalStorage()
{
}

mitk::TextOverlay3D::LocalStorage::LocalStorage()
{
  // Create some text
  m_textSource = vtkSmartPointer<vtkVectorText>::New();

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

void mitk::TextOverlay3D::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  if(ls->IsGenerateDataRequired(renderer,this))
  {
    ls->m_follower->SetPosition(
          GetPosition3D(renderer)[0]+GetOffsetVector(renderer)[0],
        GetPosition3D(renderer)[1]+GetOffsetVector(renderer)[1],
        GetPosition3D(renderer)[2]+GetOffsetVector(renderer)[2]);
    ls->m_textSource->SetText(GetText().c_str());
    float color[3] = {1,1,1};
    float opacity = 1.0;
    GetColor(color,renderer);
    GetOpacity(opacity,renderer);
    ls->m_follower->GetProperty()->SetColor(color[0], color[1], color[2]);
    ls->m_follower->GetProperty()->SetOpacity(opacity);
    ls->m_follower->SetScale(this->GetFontSize());
    ls->UpdateGenerateDataTime();
  }
}

vtkProp* mitk::TextOverlay3D::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  ls->m_follower->SetCamera(renderer->GetVtkRenderer()->GetActiveCamera());
  return ls->m_follower;
}
