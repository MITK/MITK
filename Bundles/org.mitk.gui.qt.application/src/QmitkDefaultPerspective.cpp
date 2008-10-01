/*=========================================================================

 Program:   openCherry Platform
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

#include "QmitkDefaultPerspective.h"

void QmitkDefaultPerspective::CreateInitialLayout(cherry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  cherry::IFolderLayout::Pointer bottom = layout->CreateFolder("bottom", cherry::IPageLayout::BOTTOM, 0.7, editorArea);
  bottom->AddView("org.opencherry.views.logview");
  bottom->AddView("org.mitk.views.datamanager");
}
