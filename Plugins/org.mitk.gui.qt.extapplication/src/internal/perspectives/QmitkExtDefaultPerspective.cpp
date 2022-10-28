/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkExtDefaultPerspective.h"
#include "berryIViewLayout.h"

QmitkExtDefaultPerspective::QmitkExtDefaultPerspective()
{
}

void QmitkExtDefaultPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.21f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  auto folder = layout->CreateFolder("bottomleft", berry::IPageLayout::BOTTOM, 0.72f, "org.mitk.views.datamanager");
  folder->AddView("org.mitk.views.imagenavigator");
  folder->AddView("org.mitk.views.pixelvalue");

  layout->AddView("org.mitk.views.viewnavigator", berry::IPageLayout::RIGHT, 0.7f, editorArea);

  berry::IPlaceholderFolderLayout::Pointer bottomFolder = layout->CreatePlaceholderFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddPlaceholder("org.blueberry.views.logview");
  bottomFolder->AddPlaceholder("org.mitk.views.modules");

  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.editor");
  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.visualization");
}
