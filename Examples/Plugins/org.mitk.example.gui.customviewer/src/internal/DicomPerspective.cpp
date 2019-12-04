/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "DicomPerspective.h"
#include "berryIFolderLayout.h"

DicomPerspective::DicomPerspective()
{
}

// //! [DicomPerspCreateLayout]
void DicomPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(false);
  layout->AddStandaloneView(
    "org.mitk.customviewer.views.dicomview", false, berry::IPageLayout::LEFT, 1.0f, layout->GetEditorArea());
  layout->GetViewLayout("org.mitk.customviewer.views.dicomview")->SetCloseable(false);
  layout->GetViewLayout("org.mitk.customviewer.views.dicomview")->SetMoveable(false);
}
// //! [DicomPerspCreateLayout]
