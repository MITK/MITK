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

#include "QmitkDIAppFiberTractographyPerspective.h"
#include "berryIViewLayout.h"

void QmitkDIAppFiberTractographyPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
    false, berry::IPageLayout::BOTTOM, 0.15f, "org.mitk.views.datamanager");

  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("org.mitk.diffusionimaginginternal.leftcontrols2",
    berry::IPageLayout::BOTTOM, 0.1f, "org.mitk.views.controlvisualizationpropertiesview");

  berry::IFolderLayout::Pointer bottomleft =
    layout->CreateFolder("org.mitk.diffusionimaginginternal.leftcontrols",
    berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.diffusionimaginginternal.leftcontrols2");

  layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
    berry::IPageLayout::BOTTOM, .6f, "org.mitk.diffusionimaginginternal.leftcontrols", false);

  /////////////////////////////////////////////
  // add the views
  /////////////////////////////////////////////

  left->AddView("org.mitk.views.gibbstracking");
  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.gibbstracking");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.stochasticfibertracking");
  lo = layout->GetViewLayout("org.mitk.views.stochasticfibertracking");
  lo->SetCloseable(false);

  left->AddView("org.mitk.views.streamlinetracking");
  lo = layout->GetViewLayout("org.mitk.views.streamlinetracking");
  lo->SetCloseable(false);

  bottomleft->AddView("org.mitk.views.fiberprocessing");
  berry::IViewLayout::Pointer lo2 = layout->GetViewLayout("org.mitk.views.fiberprocessing");
  lo2->SetCloseable(false);

  bottomleft->AddView("org.mitk.views.segmentation");
  lo2 = layout->GetViewLayout("org.mitk.views.segmentation");
  lo2->SetCloseable(false);
}
