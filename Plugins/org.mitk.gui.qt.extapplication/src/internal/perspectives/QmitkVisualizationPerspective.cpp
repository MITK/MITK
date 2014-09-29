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

#include "QmitkVisualizationPerspective.h"
#include "berryIViewLayout.h"

void QmitkVisualizationPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  /////////////////////////////////////////////////////
  // all di-app perspectives should have the following:
  /////////////////////////////////////////////////////

  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("org.mitk.extapplication.leftcontrols",
    berry::IPageLayout::BOTTOM, 0.1f, "org.mitk.views.datamanager");

  layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
    berry::IPageLayout::BOTTOM, .4f, "org.mitk.extapplication.leftcontrols", true);

  /////////////////////////////////////////////
  // here goes the perspective specific stuff
  /////////////////////////////////////////////

  left->AddView("org.mitk.views.volumevisualization");
  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.volumevisualization");
  lo->SetCloseable(true);

  left->AddView("org.mitk.views.screenshotmaker");
  lo = layout->GetViewLayout("org.mitk.views.screenshotmaker");
  lo->SetCloseable(true);

  left->AddView("org.mitk.views.moviemaker");
  lo = layout->GetViewLayout("org.mitk.views.moviemaker");
  lo->SetCloseable(true);
}
