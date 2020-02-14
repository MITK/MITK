/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkUSNavigationPerspective.h"

QmitkUSNavigationPerspective::QmitkUSNavigationPerspective()
{
}

void QmitkUSNavigationPerspective::CreateInitialLayout (berry::IPageLayout::Pointer layout)
{
  // place navigation plugin on the right side (not closable)
  layout->AddStandaloneView("org.mitk.views.usmarkerplacement", false, berry::IPageLayout::RIGHT, 0.75f, layout->GetEditorArea());

  // place tracking toolbox and ultrasound support on the left into a folder
  // layout (closeable)
  layout->AddStandaloneView("org.mitk.views.ultrasoundsupport", false, berry::IPageLayout::LEFT, 0.3f, layout->GetEditorArea());
  layout->AddStandaloneView("org.mitk.views.mitkigttrackingtoolbox", false, berry::IPageLayout::BOTTOM, 0.9f, "org.mitk.views.ultrasoundsupport");
}
