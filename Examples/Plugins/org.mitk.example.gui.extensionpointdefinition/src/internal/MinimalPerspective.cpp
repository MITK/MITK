/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "MinimalPerspective.h"

/// Berry
#include "berryIViewLayout.h"

MinimalPerspective::MinimalPerspective()
{
}

void MinimalPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free.
  QString editorAreaId = layout->GetEditorArea();
  // Hides the editor area.
  layout->SetEditorAreaVisible(false);

  layout->AddView("org.mitk.views.minimalview", berry::IPageLayout::LEFT, 0.5f, editorAreaId);
}
