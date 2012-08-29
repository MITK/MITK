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
#include "berryIFolderLayout.h"

//#include <QmitkDataManagerView.h>
//#include "SimpleRenderWindowView.h"

ViewerPerspective::ViewerPerspective()
{
}

void ViewerPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  //layout->SetFixed(true);
  std::string editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(false);
  layout->AddStandaloneView("org.mitk.views.datamanager", false, 1, 0.3f, layout->GetEditorArea());
  layout->AddStandaloneView("org.mitk.views.simplerenderwindowview", false, 3, 0.7f, layout->GetEditorArea());
  /*layout->AddStandaloneView(QmitkDataManagerView::VIEW_ID, false, 1, 0.3f, layout->GetEditorArea());
  layout->AddStandaloneView(SimpleRenderWindowView::View_ID, false, 3, 0.7f, layout->GetEditorArea());*/
}
