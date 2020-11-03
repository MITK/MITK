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

void ExtendedPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free.
  QString editorAreaId = layout->GetEditorArea();
  // Hides the editor area.
  layout->SetEditorAreaVisible(false);

  // add the selection view (for providing selection events for the listener view) to the perspective
  layout->AddView("org.mitk.views.selectionview", berry::IPageLayout::LEFT, 0.5f, editorAreaId);
  // add the listener view (listening for selection events of the selection view) to the perspective
  layout->AddView("org.mitk.views.listenerview", berry::IPageLayout::RIGHT, 0.5f, editorAreaId);
}
