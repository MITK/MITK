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

#include "MinimalPerspective.h"

/// Berry
#include "berryIViewLayout.h"

MinimalPerspective::MinimalPerspective()
{
}

void MinimalPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free.
  std::string editorAreaId = layout->GetEditorArea();
  // Hides the editor area.
  layout->SetEditorAreaVisible(true);

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("left", berry::IPageLayout::LEFT, 0.3f, editorAreaId);

  left->AddView("org.mitk.views.emptyview1");
  layout->AddView("org.mitk.views.emptyview2", berry::IPageLayout::RIGHT,0.3f, editorAreaId);
}