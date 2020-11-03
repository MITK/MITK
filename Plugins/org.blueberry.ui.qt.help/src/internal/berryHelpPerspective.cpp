/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHelpPerspective.h"

namespace  berry {

const QString HelpPerspective::ID = "org.blueberry.perspectives.help";

HelpPerspective::HelpPerspective()
{

}

void HelpPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.blueberry.views.helpsearch",
                  berry::IPageLayout::LEFT, 0.3f, editorArea);

  berry::IFolderLayout::Pointer leftFolder =
      layout->CreateFolder("lefttop", berry::IPageLayout::TOP, 0.65f, "org.blueberry.views.helpsearch");
  leftFolder->AddView("org.blueberry.views.helpcontents");
  leftFolder->AddView("org.blueberry.views.helpindex");

  // Make every help related view unclosable
  IViewLayout::Pointer lo = layout->GetViewLayout("org.blueberry.views.helpsearch");
  lo->SetCloseable(false);
  lo = layout->GetViewLayout("org.blueberry.views.helpcontents");
  lo->SetCloseable(false);
  lo = layout->GetViewLayout("org.blueberry.views.helpindex");
  lo->SetCloseable(false);
}

}
