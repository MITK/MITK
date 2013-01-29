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

#include "DicomPerspective.h"
#include "berryIFolderLayout.h"

DicomPerspective::DicomPerspective()
{
}

// //! [DicomPerspCreateLayout]
void DicomPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(false);
  layout->AddStandaloneView("org.mitk.customviewer.views.dicomview", false, berry::IPageLayout::LEFT, 1.0f, layout->GetEditorArea());
  layout->GetViewLayout("org.mitk.customviewer.views.dicomview")->SetCloseable(false);
  layout->GetViewLayout("org.mitk.customviewer.views.dicomview")->SetMoveable(false);
}
// //! [DicomPerspCreateLayout]
