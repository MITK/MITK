/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkExtDefaultPerspective.h"

#include <mitkWorkbenchUtil.h>

#include <berryIViewLayout.h>

QmitkExtDefaultPerspective::QmitkExtDefaultPerspective()
{
}

void QmitkExtDefaultPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  const QString editorArea = layout->GetEditorArea();

  const bool hasDataManager = mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.datamanager");
  const bool hasHelpIndex = mitk::WorkbenchUtil::IsViewAvailable("org.blueberry.views.helpindex");
  const bool hasLeftFolder = hasDataManager || hasHelpIndex;

  if (hasLeftFolder)
  {
    auto leftFolder = layout->CreateFolder("left", berry::IPageLayout::LEFT, 0.21f, editorArea);

    if (hasDataManager)
    {
      leftFolder->AddView("org.mitk.views.datamanager");
      layout->GetViewLayout("org.mitk.views.datamanager")->SetCloseable(false);
    }

    if (hasHelpIndex)
      leftFolder->AddPlaceholder("org.blueberry.views.helpindex");
  }

  const auto bottomLeftViews = mitk::WorkbenchUtil::FilterAvailableViews(
    { "org.mitk.views.imagenavigator", "org.mitk.views.pixelvalue" });

  if (!bottomLeftViews.isEmpty())
  {
    // Anchored on the folder ID instead of on the Data Manager, so it keeps resolving
    // when the Data Manager is not part of the build. Without a folder above it, this
    // becomes the left column itself.
    auto bottomLeftFolder = hasLeftFolder
      ? layout->CreateFolder("bottomleft", berry::IPageLayout::BOTTOM, 0.72f, "left")
      : layout->CreateFolder("bottomleft", berry::IPageLayout::LEFT, 0.21f, editorArea);

    for (const auto& viewId : bottomLeftViews)
      bottomLeftFolder->AddView(viewId);
  }

  if (mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.viewnavigator"))
    layout->AddView("org.mitk.views.viewnavigator", berry::IPageLayout::RIGHT, 0.62f, editorArea);

  const auto bottomViews = mitk::WorkbenchUtil::FilterAvailableViews(
    { "org.blueberry.views.logview", "org.mitk.views.modules", "org.mitk.views.pythonenvironments" });

  if (!bottomViews.isEmpty())
  {
    auto bottomFolder = layout->CreatePlaceholderFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);

    for (const auto& viewId : bottomViews)
      bottomFolder->AddPlaceholder(viewId);
  }

  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.editor");
  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.visualization");
}
