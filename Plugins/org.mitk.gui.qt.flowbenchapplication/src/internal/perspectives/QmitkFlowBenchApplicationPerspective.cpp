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

#include "QmitkFlowBenchApplicationPerspective.h"
#include "berryIViewLayout.h"

QmitkFlowBenchApplicationPerspective::QmitkFlowBenchApplicationPerspective()
{
}

void QmitkFlowBenchApplicationPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.multilabelsegmentation", berry::IPageLayout::RIGHT, 0.7f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.multilabelsegmentation");
  lo->SetCloseable(false);

  layout->AddView("org.mitk.views.imagenavigator",
    berry::IPageLayout::LEFT, 0.4f, editorArea);

  berry::IPlaceholderFolderLayout::Pointer bottomFolder = layout->CreatePlaceholderFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddPlaceholder("org.blueberry.views.logview");
  bottomFolder->AddPlaceholder("org.mitk.views.modules");

  berry::IPlaceholderFolderLayout::Pointer rightFolder = layout->CreatePlaceholderFolder("right", berry::IPageLayout::RIGHT, 0.3f, editorArea);
  rightFolder->AddPlaceholder("org.mitk.views.datamanager");

  layout->AddPerspectiveShortcut("org.mitk.qt.flowbenchapplication.defaultperspective");
}
