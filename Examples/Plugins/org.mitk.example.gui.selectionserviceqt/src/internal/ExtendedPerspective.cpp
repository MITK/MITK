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

void ExtendedPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free.
  std::string editorAreaId = layout->GetEditorArea();
  // Hides the editor area.
  layout->SetEditorAreaVisible(false);

  // add the selection view (for providing selection events for the listener view) to the perspective
  layout->AddView("org.mitk.views.selectionview", berry::IPageLayout::LEFT,0.5f, editorAreaId);
  // add the listener view (listening for selection events of the selection view) to the perspective
  layout->AddView("org.mitk.views.listenerview", berry::IPageLayout::RIGHT,0.5f, editorAreaId);
}
