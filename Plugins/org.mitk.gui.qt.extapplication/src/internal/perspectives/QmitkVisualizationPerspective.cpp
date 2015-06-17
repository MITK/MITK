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

  QString editorArea = layout->GetEditorArea();

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

  // Adding the entry for the image navigator to the Windows->"Show View" menu
  layout->AddShowViewShortcut("org.mitk.views.imagenavigator");

  layout->AddPerspectiveShortcut("org.mitk.extapp.defaultperspective");
  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.editor");
}
