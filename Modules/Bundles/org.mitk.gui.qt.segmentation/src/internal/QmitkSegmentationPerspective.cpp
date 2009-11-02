/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 19650 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkSegmentationPerspective.h"


void QmitkSegmentationPerspective::CreateInitialLayout(cherry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();
  
  layout->AddStandaloneView("org.mitk.views.segmentation", false, cherry::IPageLayout::LEFT, 0.3f, editorArea);
  layout->AddStandaloneView("org.mitk.views.datamanager", false, cherry::IPageLayout::TOP, 0.2f, "org.mitk.views.segmentation");
  layout->AddStandaloneView("org.mitk.views.perspectiveswitcher", false, cherry::IPageLayout::BOTTOM, 0.95, "org.mitk.views.segmentation");
}
