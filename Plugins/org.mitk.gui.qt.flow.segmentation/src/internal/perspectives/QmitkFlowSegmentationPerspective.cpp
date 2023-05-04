/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFlowSegmentationPerspective.h"
#include <berryIViewLayout.h>

QmitkFlowSegmentationPerspective::QmitkFlowSegmentationPerspective()
{
}

void QmitkFlowSegmentationPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.segmentation", berry::IPageLayout::LEFT, 0.24f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.segmentation");
  lo->SetCloseable(false);

  layout->AddStandaloneView("org.mitk.views.flow.control",false, berry::IPageLayout::RIGHT, 0.72f, editorArea);
  lo = layout->GetViewLayout("org.mitk.views.flow.control");
  lo->SetCloseable(false);

  layout->AddView("org.mitk.views.imagenavigator",
    berry::IPageLayout::TOP, 0.1f, "org.mitk.views.flow.control");

  berry::IPlaceholderFolderLayout::Pointer bottomFolder = layout->CreatePlaceholderFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddPlaceholder("org.blueberry.views.logview");

  berry::IPlaceholderFolderLayout::Pointer rightFolder = layout->CreatePlaceholderFolder("right", berry::IPageLayout::RIGHT, 0.3f, editorArea);
  rightFolder->AddPlaceholder("org.mitk.views.datamanager");

  layout->AddPerspectiveShortcut("org.mitk.qt.flowapplication.defaultperspective");
}
