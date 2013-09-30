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

#include "QmitkDIAppRegistrationPerspective.h"
#include "berryIViewLayout.h"

void QmitkDIAppRegistrationPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  /////////////////////////////////////////////////////
  // all di-app perspectives should have the following:
  /////////////////////////////////////////////////////

  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
    false, berry::IPageLayout::BOTTOM, .15f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("org.mbi.diffusionimaginginternal.leftcontrols",
    berry::IPageLayout::BOTTOM, 0.1f, "org.mitk.views.controlvisualizationpropertiesview");

  layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
    berry::IPageLayout::BOTTOM, .4f, "org.mbi.diffusionimaginginternal.leftcontrols", false);

  /////////////////////////////////////////////
  // here goes the perspective specific stuff
  /////////////////////////////////////////////

  left->AddView("org.mitk.views.rigidregistration");
  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.rigidregistration");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.pointbasedregistration");
  lo = layout->GetViewLayout("org.mitk.views.pointbasedregistration");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.diffusionregistrationview");
  lo = layout->GetViewLayout("org.mitk.views.diffusionregistrationview");
  lo->SetCloseable(false);
}
