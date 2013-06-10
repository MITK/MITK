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


void mitk::TextOverlay2D::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  ls->m_textActor->GetTextProperty()->SetFontSize ( 24 );
  ls->m_textActor->SetPosition2 ( 10, 40 );
  ls->m_textActor->SetInput ( "Hello world" );
  ls->m_textActor->GetTextProperty()->SetColor ( 1.0,0.0,0.0 );
}

vtkSmartPointer<vtkActor2D> mitk::TextOverlay2D::GetVtkActor2D(BaseRenderer *renderer)
{

}

