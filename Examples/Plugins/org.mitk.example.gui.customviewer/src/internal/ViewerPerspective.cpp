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

#include "ViewerPerspective.h"
#include "berryIFolderLayout.h"

ViewerPerspective::ViewerPerspective()
{
}

// //! [AddView1]
void ViewerPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(false);
  layout->AddStandaloneView("org.mitk.views.datamanager", false, berry::IPageLayout::LEFT, 0.3f, layout->GetEditorArea());
  layout->AddStandaloneView("org.mitk.customviewer.views.simplerenderwindowview", false, berry::IPageLayout::RIGHT, 0.7f, layout->GetEditorArea());
}
// //! [AddView1]
