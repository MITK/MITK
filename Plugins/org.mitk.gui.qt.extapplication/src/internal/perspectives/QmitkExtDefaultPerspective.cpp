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

#include "QmitkExtDefaultPerspective.h"
#include "berryIViewLayout.h"

QmitkExtDefaultPerspective::QmitkExtDefaultPerspective()
{
}

void QmitkExtDefaultPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.3f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  layout->AddView("org.mitk.views.imagenavigator",
    berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");

  berry::IPlaceholderFolderLayout::Pointer bottomFolder = layout->CreatePlaceholderFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddPlaceholder("org.blueberry.views.logview");
  bottomFolder->AddPlaceholder("org.mitk.views.modules");
}
