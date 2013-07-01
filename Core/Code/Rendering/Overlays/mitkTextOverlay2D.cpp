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


mitk::TextOverlay2D::TextOverlay2D()
{
}


mitk::TextOverlay2D::~TextOverlay2D()
{
}

mitk::TextOverlay2D::LocalStorage::~LocalStorage()
{
}

mitk::TextOverlay2D::LocalStorage::LocalStorage()
{
  m_textActor = vtkSmartPointer<vtkTextActor>::New();
}

void mitk::TextOverlay2D::UpdateVtkOverlay2D(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  ls->m_textActor->GetTextProperty()->SetFontSize ( 24 );
  ls->m_textActor->SetDisplayPosition( GetPosition2D(renderer)[0]+GetOffsetVector(renderer)[0], GetPosition2D(renderer)[1]+GetOffsetVector(renderer)[1] );
  ls->m_textActor->SetInput ( GetText().c_str());
  float color[3] = {1,1,1};
  float opacity = 1.0;
  GetColor(color,renderer);
  GetOpacity(opacity,renderer);
  ls->m_textActor->GetTextProperty()->SetColor ( color[0], color[1], color[2]);
  ls->m_textActor->GetTextProperty()->SetOpacity(opacity);
}

vtkSmartPointer<vtkActor2D> mitk::TextOverlay2D::GetVtkActor2D(BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_textActor;
}

