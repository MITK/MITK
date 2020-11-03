/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "ExtendedPerspective.h"

// berry includes
#include "berryIViewLayout.h"

ExtendedPerspective::ExtendedPerspective()
{
}

void ExtendedPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free
  QString editorAreaId = layout->GetEditorArea();

  // Hide editor area
  layout->SetEditorAreaVisible(false);

  // add emptyviews to perspective
  layout->AddView("org.mitk.views.emptyview1", berry::IPageLayout::LEFT, 1.0f, editorAreaId);
  layout->AddView("org.mitk.views.emptyview2", berry::IPageLayout::RIGHT, 0.3f, "org.mitk.views.emptyview1");
}
