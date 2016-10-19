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
#include <vtkCamera.h>
#include <vtkMath.h>


mitk::TextOverlay3D::TextOverlay3D()
{
  mitk::Point3D position;
  position.Fill(0);
  this->SetPosition3D(position);
  this->SetOffsetVector(position);
  this->SetText("");
  this->SetFontSize(20);
  this->SetColor(1.0,1.0,1.0);
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
    Point3D pos3d = GetPosition3D(renderer);
    vtkRenderer* vtkRender = renderer->GetVtkRenderer();
    if(vtkRender)
    {
      vtkCamera* camera = vtkRender->GetActiveCamera();
      ls->m_follower->SetCamera(camera);
      if (camera != nullptr)
      {
        // calculate the offset relative to the camera's view direction
        Point3D offset = GetOffsetVector(renderer);

        Vector3D viewUp;
        camera->GetViewUp(viewUp.GetDataPointer());
        Vector3D cameraDirection;
        camera->GetDirectionOfProjection(cameraDirection.GetDataPointer());
        Vector3D viewRight;
        vtkMath::Cross(cameraDirection.GetDataPointer(),
                       viewUp.GetDataPointer(),
                       viewRight.GetDataPointer());

        pos3d = pos3d + viewRight * offset[0]
                      + viewUp * offset[1]
                      + cameraDirection * offset[2];
      }
    }
    ls->m_follower->SetPosition(pos3d.GetDataPointer());
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
  return ls->m_follower;
}
