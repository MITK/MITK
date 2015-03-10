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
  // Editors are placed for free.
  QString editorAreaId = layout->GetEditorArea();
  // Hides the editor area.
  layout->SetEditorAreaVisible(false);

  // add the selection view (for providing selection events for the listener view) to the perspective
  layout->AddView("org.mitk.views.selectionviewmitk", berry::IPageLayout::LEFT,1.0f, editorAreaId);
  // add the listener view (listening for selection events of the selection view) to the perspective
  layout->AddView("org.mitk.views.listenerviewmitk", berry::IPageLayout::RIGHT,0.5f, "org.mitk.views.selectionviewmitk");
}
