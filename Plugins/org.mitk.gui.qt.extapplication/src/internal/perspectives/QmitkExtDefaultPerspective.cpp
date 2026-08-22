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

  berry::IFolderLayout::Pointer leftFolder;

  if (mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.datamanager"))
  {
    leftFolder = layout->CreateFolder("left", berry::IPageLayout::LEFT, 0.21f, editorArea);
    leftFolder->AddView("org.mitk.views.datamanager");
    layout->GetViewLayout("org.mitk.views.datamanager")->SetCloseable(false);
  }

  const auto bottomLeftViews = mitk::WorkbenchUtil::FilterAvailableViewIds(
    { "org.mitk.views.imagenavigator", "org.mitk.views.pixelvalue" });

  berry::IFolderLayout::Pointer bottomLeftFolder;

  if (!bottomLeftViews.isEmpty())
  {
    // Anchored on the folder ID instead of on the Data Manager, so it keeps resolving
    // when the Data Manager is not part of the build. Without a folder above it, this
    // becomes the left column itself.
    bottomLeftFolder = leftFolder.IsNotNull()
      ? layout->CreateFolder("bottomleft", berry::IPageLayout::BOTTOM, 0.72f, "left")
      : layout->CreateFolder("bottomleft", berry::IPageLayout::LEFT, 0.21f, editorArea);

    for (const auto& viewId : bottomLeftViews)
      bottomLeftFolder->AddView(viewId);
  }

  // A placeholder only reserves the spot a view takes once it is opened, so it has to
  // join a folder that holds views of its own. A folder is laid out whether or not it
  // has content, so one created for nothing but a placeholder leaves an empty panel.
  const auto helpIndexHost = leftFolder.IsNotNull() ? leftFolder : bottomLeftFolder;

  if (helpIndexHost.IsNotNull() && mitk::WorkbenchUtil::IsViewAvailable("org.blueberry.views.helpindex"))
    helpIndexHost->AddPlaceholder("org.blueberry.views.helpindex");

  if (mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.viewnavigator"))
    layout->AddView("org.mitk.views.viewnavigator", berry::IPageLayout::RIGHT, 0.62f, editorArea);

  const auto bottomViews = mitk::WorkbenchUtil::FilterAvailableViewIds(
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
