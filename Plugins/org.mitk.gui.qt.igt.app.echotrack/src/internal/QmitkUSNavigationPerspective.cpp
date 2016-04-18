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
#include "QmitkUSNavigationPerspective.h"

QmitkUSNavigationPerspective::QmitkUSNavigationPerspective()
{
}

void QmitkUSNavigationPerspective::CreateInitialLayout (berry::IPageLayout::Pointer layout)
{
  // place navigation plugin on the right side (not closable)
  layout->AddStandaloneView("org.mitk.views.usmarkerplacement", false, berry::IPageLayout::RIGHT, 0.8f, layout->GetEditorArea());

  // place tracking toolbox and ultrasound support on the left into a folder
  // layout (closeable)
  berry::IFolderLayout::Pointer leftFolder = layout->CreateFolder("left", berry::IPageLayout::LEFT, 0.3f, layout->GetEditorArea());

  leftFolder->AddView("org.mitk.views.mitkigttrackingtoolbox");
  leftFolder->AddView("org.mitk.views.ultrasoundsupport");

  layout->GetViewLayout("org.mitk.views.mitkigttrackingtoolbox")->SetCloseable(false);
  layout->GetViewLayout("org.mitk.views.ultrasoundsupport")->SetCloseable(false);
}
