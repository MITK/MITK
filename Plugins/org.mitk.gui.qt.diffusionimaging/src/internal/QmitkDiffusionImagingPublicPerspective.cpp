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

#include "QmitkDiffusionImagingPublicPerspective.h"
#include "berryIViewLayout.h"

QmitkDiffusionImagingPublicPerspective::QmitkDiffusionImagingPublicPerspective()
{

}

QmitkDiffusionImagingPublicPerspective::QmitkDiffusionImagingPublicPerspective(const QmitkDiffusionImagingPublicPerspective& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void QmitkDiffusionImagingPublicPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
    false, berry::IPageLayout::BOTTOM, .2f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("org.mitk.leftcontrols",
    berry::IPageLayout::BOTTOM, 0.125f, "org.mitk.views.controlvisualizationpropertiesview");

  layout->AddStandaloneView("org.mitk.views.imagenavigator",
    false, berry::IPageLayout::BOTTOM, 0.8f, "org.mitk.leftcontrols");

  layout->AddStandaloneView("org.mitk.views.perspectiveswitcher",
    false, berry::IPageLayout::BOTTOM, 0.99f, "org.mitk.views.imagenavigator");

//  layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
//    berry::IPageLayout::TOP, 0.9f, "org.mitk.views.perspectiveswitcher",false);

  left->AddView("org.mitk.views.diffusionpreprocessing");
  left->AddView("org.mitk.views.tensorreconstruction");
  left->AddView("org.mitk.views.qballreconstruction");
  left->AddView("org.mitk.views.diffusionquantification");
  left->AddView("org.mitk.views.diffusiondicomimport");

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.diffusionpreprocessing");
  lo->SetCloseable(false);

  lo = layout->GetViewLayout("org.mitk.views.qballreconstruction");
  lo->SetCloseable(false);

  lo = layout->GetViewLayout("org.mitk.views.tensorreconstruction");
  lo->SetCloseable(false);

  lo = layout->GetViewLayout("org.mitk.views.diffusionquantification");
  lo->SetCloseable(false);

  lo = layout->GetViewLayout("org.mitk.views.diffusiondicomimport");
  lo->SetCloseable(false);

  //berry::IFolderLayout::Pointer right2 =
  //  layout->CreateFolder("right2", berry::IPageLayout::BOTTOM, 0.5f, "right");

}
