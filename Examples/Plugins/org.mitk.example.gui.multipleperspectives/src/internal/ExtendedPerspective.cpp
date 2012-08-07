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

#include "ExtendedPerspective.h"
#include "berryIViewLayout.h"

ExtendedPerspective::ExtendedPerspective()
{
}

void ExtendedPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free.
  std::string editorAreaId = layout->GetEditorArea();
  //! [Visibility of editor area]
  // Hides the editor area.
  layout->SetEditorAreaVisible(false);
  //! [Visibility of editor area]

  //berry::IFolderLayout::Pointer left =
  //  layout->CreateFolder("left", berry::IPageLayout::LEFT, 0.3f, editorAreaId);
  //left->AddView("org.mitk.views.datamanager");

  //layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.3f, editorAreaId);
  layout->AddView("org.mitk.views.selectionview", berry::IPageLayout::LEFT,0.3f, editorAreaId);

  layout->AddView("org.mitk.views.minimalview", berry::IPageLayout::RIGHT,0.3f, editorAreaId);

  //layout->AddView("org.mitk.views.modules", berry::IPageLayout::RIGHT,0.3f, editorAreaId);
  //layout->AddView("org.mitk.views.rigidregistration", berry::IPageLayout::RIGHT,0.3f, editorAreaId);
  //layout->AddView("org.mitk.views.mitkigttrackingtoolbox", berry::IPageLayout::RIGHT,0.3f, editorAreaId);
  //layout->AddView("org.mitk.views.imagecropper", berry::IPageLayout::RIGHT,0.3f, editorAreaId);
  //layout->AddView("org.mitk.views.imagenavigator", berry::IPageLayout::RIGHT,0.3f, editorAreaId);
}