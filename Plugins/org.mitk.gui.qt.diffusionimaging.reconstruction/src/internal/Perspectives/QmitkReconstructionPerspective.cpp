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

#include "QmitkReconstructionPerspective.h"
#include "berryIViewLayout.h"

void QmitkReconstructionPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  /////////////////////////////////////////////////////
  // all di-app perspectives should have the following:
  /////////////////////////////////////////////////////

  QString editorArea = layout->GetEditorArea();

  layout->AddStandaloneViewPlaceholder("org.mitk.views.viewnavigatorview", berry::IPageLayout::LEFT, 0.3f, editorArea, false);

  layout->AddStandaloneView("org.mitk.views.datamanager",
                            false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
                            false, berry::IPageLayout::BOTTOM, .15f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer left =
      layout->CreateFolder("org.mbi.diffusionimaginginternal.leftcontrols",
                           berry::IPageLayout::BOTTOM, 0.15f, "org.mitk.views.controlvisualizationpropertiesview");

  layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
                                       berry::IPageLayout::BOTTOM, .7f, "org.mbi.diffusionimaginginternal.leftcontrols", false);

  /////////////////////////////////////////////
  // here goes the perspective specific stuff
  /////////////////////////////////////////////

  left->AddView("org.mitk.views.tensorreconstruction");
  left->AddView("org.mitk.views.qballreconstruction");
  left->AddView("org.mitk.views.odfmaximaextraction");
  left->AddView("org.mitk.views.dipyreconstruction");
  left->AddView("org.mitk.views.odfdetails");
}
