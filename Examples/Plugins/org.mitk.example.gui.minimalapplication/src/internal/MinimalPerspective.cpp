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

#include "MinimalPerspective.h"

// Berry
#include "berryIViewLayout.h"

MinimalPerspective::MinimalPerspective()
{
}

void MinimalPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  // Hides the editor area.
  layout->SetEditorAreaVisible(false);
}
