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
  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
    false, berry::IPageLayout::BOTTOM, .15f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("org.mitk.views.leftcontrols2",
    berry::IPageLayout::BOTTOM, 0.1f, "org.mitk.views.controlvisualizationpropertiesview");

  berry::IFolderLayout::Pointer bottomleft =
    layout->CreateFolder("org.mitk.views.leftcontrols",
    berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.leftcontrols2");

  layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
    berry::IPageLayout::BOTTOM, .6f, "org.mitk.views.leftcontrols", false);

  /////////////////////////////////////////////
  // add the views
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

  bottomleft->AddView("org.mitk.views.diffusionquantification");
  berry::IViewLayout::Pointer lo2 = layout->GetViewLayout("org.mitk.views.diffusionquantification");
  lo2->SetCloseable(false);

  bottomleft->AddView("org.mitk.views.odfmaximaextraction");
  layout->GetViewLayout("org.mitk.views.odfmaximaextraction");
  lo2->SetCloseable(false);

  bottomleft->AddView("org.mitk.views.odfdetails");
  lo2 = layout->GetViewLayout("org.mitk.views.odfdetails");
  lo2->SetCloseable(false);
}
