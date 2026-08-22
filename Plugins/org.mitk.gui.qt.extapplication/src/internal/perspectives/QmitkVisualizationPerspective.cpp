/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVisualizationPerspective.h"

#include <mitkWorkbenchUtil.h>

void QmitkVisualizationPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  const QString editorArea = layout->GetEditorArea();

  const bool hasDataManager = mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.datamanager");

  if (hasDataManager)
  {
    layout->AddStandaloneView("org.mitk.views.datamanager",
      false, berry::IPageLayout::LEFT, 0.3f, editorArea);
  }

  if (mitk::WorkbenchUtil::IsViewAvailable("org.mitk.views.imagenavigator"))
  {
    // Anchored on the Data Manager, or on the editor area if that view is not part of
    // the build. The placeholder is invisible until the image navigator is opened, so
    // the ratio only takes effect from that point on.
    if (hasDataManager)
    {
      layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
        berry::IPageLayout::BOTTOM, 0.72f, "org.mitk.views.datamanager", true);
    }
    else
    {
      layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
        berry::IPageLayout::LEFT, 0.3f, editorArea, true);
    }

    // Adding the entry for the image navigator to the Windows->"Show View" menu
    layout->AddShowViewShortcut("org.mitk.views.imagenavigator");
  }

  layout->AddPerspectiveShortcut("org.mitk.extapp.defaultperspective");
  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.editor");
}
