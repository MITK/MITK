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

#include "QmitkExtDefaultPerspective.h"


void QmitkExtDefaultPerspective::CreateInitialLayout(cherry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", cherry::IPageLayout::LEFT, 0.3f, editorArea);
  cherry::IFolderLayout::Pointer bottomFolder = layout->CreateFolder("bottom", cherry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddView("org.mitk.views.propertylistview");
  bottomFolder->AddView("org.opencherry.views.logview");
}
