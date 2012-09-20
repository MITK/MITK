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

#include "QmitkDIAppPreprocessingReconstructionPerspective.h"
#include "berryIViewLayout.h"

void QmitkDIAppPreprocessingReconstructionPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  /////////////////////////////////////////////////////
  // all di-app perspectives should have the following:
  /////////////////////////////////////////////////////

  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
    false, berry::IPageLayout::BOTTOM, .2f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("org.mbi.diffusionimaginginternal.leftcontrols",
    berry::IPageLayout::BOTTOM, 0.15f, "org.mitk.views.controlvisualizationpropertiesview");

  layout->AddStandaloneView("org.mitk.views.imagenavigator",
    false, berry::IPageLayout::BOTTOM, .4f, "org.mbi.diffusionimaginginternal.leftcontrols");

  /////////////////////////////////////////////
  // here goes the perspective specific stuff
  /////////////////////////////////////////////

  left->AddView("org.mitk.views.diffusionpreprocessing");
  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.diffusionpreprocessing");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.tensorreconstruction");
  lo = layout->GetViewLayout("org.mitk.views.tensorreconstruction");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.qballreconstruction");
  lo = layout->GetViewLayout("org.mitk.views.qballreconstruction");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.odfdetails");
  lo = layout->GetViewLayout("org.mitk.views.odfdetails");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.odfmaximaextraction");
  lo = layout->GetViewLayout("org.mitk.views.odfmaximaextraction");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.dwisoftwarephantomview");
  lo = layout->GetViewLayout("org.mitk.views.dwisoftwarephantomview");
  lo->SetCloseable(false);

}
