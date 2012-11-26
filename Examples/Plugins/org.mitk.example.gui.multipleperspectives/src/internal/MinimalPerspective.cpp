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

#include "MinimalPerspective.h"

// berry includes
#include "berryIViewLayout.h"

MinimalPerspective::MinimalPerspective()
{
}

void MinimalPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Editors are placed for free.
  std::string editorAreaId = layout->GetEditorArea();

  //! [Visibility of editor area]
  // set editor area to visible
  layout->SetEditorAreaVisible(true);
  //! [Visibility of editor area]

  // create folder with alignment "left"
  berry::IFolderLayout::Pointer left =
    layout->CreateFolder("left", berry::IPageLayout::LEFT, 0.5f, editorAreaId);

  // add emptyview1 to the folder
  left->AddView("org.mitk.views.emptyview1");
  // add emptyview2 to the folder
  left->AddView("org.mitk.views.emptyview2");
}
