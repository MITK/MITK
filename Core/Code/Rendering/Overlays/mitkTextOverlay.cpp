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

#include "mitkTextOverlay.h"

mitk::TextOverlay::TextOverlay()
{
}


mitk::TextOverlay::~TextOverlay()
{
}

mitk::TextOverlay::LocalStorage::~LocalStorage()
{
}

mitk::TextOverlay::LocalStorage::LocalStorage()
{
  vtkSmartPointer<vtkTextActor> m_textActor = vtkSmartPointer<vtkTextActor>::New();
  m_textActor->GetTextProperty()->SetFontSize ( 24 );
  m_textActor->SetPosition2 ( 10, 40 );
  renderer->AddActor2D ( m_textActor );
  m_textActor->SetInput ( "Hello world" );
  m_textActor->GetTextProperty()->SetColor ( 1.0,0.0,0.0 );
}
