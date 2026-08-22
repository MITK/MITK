/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVisualizationPerspective.h"

#include <mitkWorkbenchUtil.h>

#include <berryIViewLayout.h>

void QmitkVisualizationPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  const QString editorArea = layout->GetEditorArea();

  // The left column only exists to host the Data Manager, so it is skipped
  // entirely if that view is not part of the build.
  if (mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.datamanager"))
  {
    layout->AddStandaloneView("org.mitk.views.datamanager",
      false, berry::IPageLayout::LEFT, 0.3f, editorArea);

    if (mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.imagenavigator"))
    {
      // The folder stays empty on purpose. It only provides the anchor for the
      // image navigator placeholder below the Data Manager.
      layout->CreateFolder("org.mitk.extapplication.leftcontrols",
        berry::IPageLayout::BOTTOM, 0.1f, "org.mitk.views.datamanager");

      layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
        berry::IPageLayout::BOTTOM, .4f, "org.mitk.extapplication.leftcontrols", true);

      // Adding the entry for the image navigator to the Windows->"Show View" menu
      layout->AddShowViewShortcut("org.mitk.views.imagenavigator");
    }
  }

  layout->AddPerspectiveShortcut("org.mitk.extapp.defaultperspective");
  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.editor");
}
