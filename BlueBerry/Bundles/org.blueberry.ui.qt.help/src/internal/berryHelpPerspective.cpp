/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

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
}

}
