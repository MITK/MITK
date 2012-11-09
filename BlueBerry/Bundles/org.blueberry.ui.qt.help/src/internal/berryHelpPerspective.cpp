/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryHelpPerspective.h"

namespace  berry {

const std::string HelpPerspective::ID = "org.blueberry.perspectives.help";

HelpPerspective::HelpPerspective()
{

}

void HelpPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddView("org.blueberry.views.helpsearch",
                  berry::IPageLayout::LEFT, 0.3, editorArea);

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
