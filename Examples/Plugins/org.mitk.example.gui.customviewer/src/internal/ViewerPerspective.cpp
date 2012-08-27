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

#include "ViewerPerspective.h"
//#include "DicomView.h"
//#include "SimpleRenderWindowView.h"
//#include "TestView.h"
#include "berryIFolderLayout.h"

ViewerPerspective::ViewerPerspective()
{
}

void ViewerPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  //layout->SetFixed(true);
  std::string editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(false);
  //layout->AddStandaloneView("org.mitk.views.testview", false, 1, 1.0f, layout->GetEditorArea());
  layout->AddStandaloneView("org.mitk.views.datamanager", false, 1, 0.3f, layout->GetEditorArea());
  //layout->AddView("org.mitk.views.datamanager", false, 1.0f, editorArea);
  //layout->GetViewLayout("org.mitk.views.datamanager")->SetCloseable(false);
  //layout->GetViewLayout("org.mitk.views.datamanager")->SetMoveable(false);
  layout->AddStandaloneView("org.mitk.views.simplerenderwindowview", false, 3, 0.7f, layout->GetEditorArea());

  ////layout->AddStandaloneView(TestView::VIEW_ID,true,2,0.33f, layout->GetEditorArea());
  //layout->AddView(TestView::VIEW_ID, 2, 0.33f, editorArea);
  //layout->GetViewLayout(TestView::VIEW_ID)->SetCloseable(false);
  

  //berry::IFolderLayout::Pointer topLeft = layout->CreateFolder("topLeft", berry::IPageLayout::LEFT, 0.25f,
  //  editorArea);
  //topLeft->AddView(berry::IPageLayout::ID_RES_NAV);
  //topLeft->AddPlaceholder(berry::IPageLayout::ID_BOOKMARKS);
  //
  //berry::IFolderLayout::Pointer bottomLeft = layout->CreateFolder("bottomLeft", berry::IPageLayout::BOTTOM, 0.50f,
  //   "topLeft");
  //bottomLeft->AddView(berry::IPageLayout::ID_OUTLINE);
  //bottomLeft->AddView(berry::IPageLayout::ID_PROP_SHEET);

 // Bottom right: Task List view
 // layout->AddView(berry::IPageLayout::ID_TASK_LIST, berry::IPageLayout::BOTTOM, 0.66f, editorArea);
 
}
