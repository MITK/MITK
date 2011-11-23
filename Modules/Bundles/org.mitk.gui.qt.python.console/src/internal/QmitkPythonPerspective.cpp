/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkPythonPerspective.h"
#include "berryIViewLayout.h"


void QmitkPythonPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", 
    berry::IPageLayout::LEFT, 0.3f, editorArea);
  layout->AddView("org.mitk.views.pythoncommandhistory",
    berry::IPageLayout::TOP, 0.3f, "org.mitk.views.datamanager");

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  berry::IFolderLayout::Pointer leftbottomFolder = layout->CreateFolder("leftbottom", berry::IPageLayout::BOTTOM, 0.7f, "org.mitk.views.datamanager");
  leftbottomFolder->AddView("org.mitk.views.pythonvariablestack");
  leftbottomFolder->AddView("org.mitk.views.pythonsnippets");
  leftbottomFolder->AddView("org.mitk.views.imagenavigator");

  berry::IFolderLayout::Pointer bottomFolder = layout->CreateFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddView("org.mitk.views.pythonconsole");
  bottomFolder->AddView("org.mitk.views.propertylistview");
  bottomFolder->AddView("org.blueberry.views.logview");
}
