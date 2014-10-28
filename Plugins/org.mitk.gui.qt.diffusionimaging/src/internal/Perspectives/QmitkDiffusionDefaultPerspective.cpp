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

#include "QmitkDiffusionDefaultPerspective.h"
#include "berryIViewLayout.h"

void QmitkDiffusionDefaultPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
    /////////////////////////////////////////////////////
    // all di-app perspectives should have the following:
    /////////////////////////////////////////////////////

    std::string editorArea = layout->GetEditorArea();

    layout->AddStandaloneView("org.mitk.views.viewnavigatorview", false, berry::IPageLayout::LEFT, 0.25f, editorArea);

    layout->AddStandaloneView("org.mitk.views.datamanager", false, berry::IPageLayout::LEFT, 0.3f, editorArea);

    layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview", false, berry::IPageLayout::BOTTOM, .15f, "org.mitk.views.datamanager");

    berry::IFolderLayout::Pointer left =
            layout->CreateFolder("org.mbi.diffusionimaginginternal.leftcontrols", berry::IPageLayout::BOTTOM, 0.1f, "org.mitk.views.controlvisualizationpropertiesview");

    layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator", berry::IPageLayout::BOTTOM, .7f, "org.mbi.diffusionimaginginternal.leftcontrols", false);

    /////////////////////////////////////////////
    // here goes the perspective specific stuff
    /////////////////////////////////////////////

}
