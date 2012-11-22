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

#include "ExtendedPerspective.h"

// berry includes
#include "berryIViewLayout.h"

ExtendedPerspective::ExtendedPerspective()
{
}

void ExtendedPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free
  std::string editorAreaId = layout->GetEditorArea();

  // Hide editor area
  layout->SetEditorAreaVisible(false);

  // add emptyviews to perspective
  layout->AddView("org.mitk.views.emptyview1", berry::IPageLayout::LEFT, 1.0f, editorAreaId);
  layout->AddView("org.mitk.views.emptyview2", berry::IPageLayout::RIGHT, 0.3f, "org.mitk.views.emptyview1");
}
