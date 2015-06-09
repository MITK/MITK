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

#include "mitkTextOverlay2D.h"
#include <vtkTextProperty.h>
#include "vtkUnicodeString.h"
#include <vtkTextActor.h>
#include <vtkPropAssembly.h>

mitk::TextOverlay2D::TextOverlay2D()
{
  mitk::Point2D position;
  position[0] = position[1] = 0;
  SetPosition2D(position);
  SetOffsetVector(position);
}


mitk::TextOverlay2D::~TextOverlay2D()
{
}

mitk::Overlay::Bounds mitk::TextOverlay2D::GetBoundsOnDisplay(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  mitk::Overlay::Bounds bounds;
  bounds.Position = ls->m_TextActor->GetPosition();

  double size[2];
  ls->m_TextActor->GetSize(renderer->GetVtkRenderer(), size);
  bounds.Size[0] = size[0];
  bounds.Size[1] = size[1];
  return bounds;
}

void mitk::TextOverlay2D::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, const mitk::Overlay::Bounds& bounds)
{
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  actor->SetDisplayPosition(bounds.Position[0],bounds.Position[1]);
}


mitk::TextOverlay2D::LocalStorage::~LocalStorage()
{
}

mitk::TextOverlay2D::LocalStorage::LocalStorage()
{
  m_TextActor = vtkSmartPointer<vtkTextActor>::New();
  m_TextProp = vtkSmartPointer<vtkTextProperty>::New();
  m_STextActor = vtkSmartPointer<vtkTextActor>::New();
  m_STextProp = vtkSmartPointer<vtkTextProperty>::New();
  m_TextActor->SetTextProperty( m_TextProp );
  m_STextActor->SetTextProperty( m_STextProp );
  m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
  m_Assembly->AddPart(m_STextActor);
  m_Assembly->AddPart(m_TextActor);
}

void mitk::TextOverlay2D::UpdateVtkOverlay2D(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);

  if(ls->IsGenerateDataRequired(renderer,this))
  {
    float color[3] = {0.0,1.0,0.0};
    float opacity = 1.0;
    GetColor(color,renderer);
    GetOpacity(opacity,renderer);
    ls->m_TextProp->SetColor( color[0], color[1], color[2]);
    ls->m_STextProp->SetColor( 0,0,0 );
    ls->m_TextProp->SetFontSize(GetFontSize());
    ls->m_TextProp->SetOpacity(opacity);
    ls->m_STextProp->SetFontSize(GetFontSize());
    ls->m_STextProp->SetOpacity(opacity);

    bool drawShadow;
    GetBoolProperty( "drawShadow", drawShadow );
    ls->m_TextProp->SetShadow( false );
    ls->m_STextProp->SetShadow( false );
    ls->m_STextActor->SetVisibility(drawShadow);

    ls->m_TextActor->SetInput( GetText().c_str() );
    ls->m_STextActor->SetInput( GetText().c_str() );

    mitk::Point2D posT, posS;
    posT[0] = GetPosition2D(renderer)[0]+GetOffsetVector(renderer)[0];
    posT[1] = GetPosition2D(renderer)[1]+GetOffsetVector(renderer)[1];
    posS[0] = posT[0]+1;
    posS[1] = posT[1]-1;
    posT = TransformDisplayPointToViewport(posT,renderer);
    posS = TransformDisplayPointToViewport(posS,renderer);

    ls->m_TextActor->SetPosition(posT[0],posT[1]);
    ls->m_STextActor->SetPosition(posS[0],posS[1]);
    ls->UpdateGenerateDataTime();
  }

}

vtkProp *mitk::TextOverlay2D::GetVtkProp(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_Assembly;
}

vtkActor2D* mitk::TextOverlay2D::GetVtkActor2D(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_TextActor;
}
