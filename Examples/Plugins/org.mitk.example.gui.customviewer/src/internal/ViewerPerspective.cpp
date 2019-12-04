/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "ViewerPerspective.h"
#include "berryIFolderLayout.h"

ViewerPerspective::ViewerPerspective()
{
}

// //! [AddView1]
void ViewerPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(false);
  layout->AddStandaloneView(
    "org.mitk.views.datamanager", false, berry::IPageLayout::LEFT, 0.3f, layout->GetEditorArea());
  layout->AddStandaloneView("org.mitk.customviewer.views.simplerenderwindowview",
                            false,
                            berry::IPageLayout::RIGHT,
                            0.7f,
                            layout->GetEditorArea());
}
// //! [AddView1]
