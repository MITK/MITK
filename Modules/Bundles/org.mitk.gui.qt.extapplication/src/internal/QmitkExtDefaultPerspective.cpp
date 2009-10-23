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
  layout->AddView("org.mitk.views.propertylistview", cherry::IPageLayout::BOTTOM, 0.3f, "org.mitk.views.datamanager");
  layout->AddView("org.opencherry.views.logview", cherry::IPageLayout::BOTTOM, 0.7f, editorArea);
}
