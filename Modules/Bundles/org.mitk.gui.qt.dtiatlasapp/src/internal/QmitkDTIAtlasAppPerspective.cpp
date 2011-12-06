/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date: 2009-10-23 02:59:36 +0200 (Fr, 23 Okt 2009) $
 Version:   $Revision: 19652 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "QmitkDTIAtlasAppPerspective.h"
#include "berryIViewLayout.h"

void QmitkDTIAtlasAppPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  std::string editorArea = layout->GetEditorArea();

  layout->AddStandaloneView("org.mitk.views.datamanager",
    false, berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
    false, berry::IPageLayout::BOTTOM, .5f, "org.mitk.views.datamanager");

  layout->AddStandaloneView("org.mitk.views.imagenavigator",
    false, berry::IPageLayout::BOTTOM, .8f, "org.mitk.views.controlvisualizationpropertiesview");


//  berry::IFolderLayout::Pointer left =
//    layout->CreateFolder("org.mbi.diffusionimaginginternal.leftcontrols",
//    berry::IPageLayout::BOTTOM, 0.2f, "org.mitk.views.controlvisualizationpropertiesview");

//  layout->AddStandaloneView("org.mitk.views.perspectiveswitcher",
//    false, berry::IPageLayout::BOTTOM, 0.99f, "org.mbi.diffusionimaginginternal.leftcontrols");

  ////////////////////////////////////////
  // public views go here
  ////////////////////////////////////////
//  left->AddView("org.mitk.views.fiberbundleoperations");
//  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.fiberbundleoperations");
//  lo->SetCloseable(false);

//  left->AddView("org.mitk.views.diffusionpreprocessing");
//  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.diffusionpreprocessing");
//  lo->SetCloseable(true);

//  left->AddView("org.mitk.views.tensorreconstruction");
//  lo = layout->GetViewLayout("org.mitk.views.tensorreconstruction");
//  lo->SetCloseable(true);

//  left->AddView("org.mitk.views.qballreconstruction");
//  lo = layout->GetViewLayout("org.mitk.views.qballreconstruction");
//  lo->SetCloseable(true);

//  left->AddView("org.mitk.views.diffusionquantification");
//  lo = layout->GetViewLayout("org.mitk.views.diffusionquantification");
//  lo->SetCloseable(true);

//  left->AddView("org.mitk.views.diffusiondicomimport");
//  lo = layout->GetViewLayout("org.mitk.views.diffusiondicomimport");
//  lo->SetCloseable(true);

  ////////////////////////////////////////
  // end public views
  ////////////////////////////////////////


//  berry::IFolderLayout::Pointer right =
//    layout->CreateFolder("org.mbi.diffusionimaginginternal.rightcontrols", berry::IPageLayout::RIGHT, 0.5f, editorArea);

  ////////////////////////////////////////
  // internal views go here
  ////////////////////////////////////////
//  right->AddView("org.mitk.views.globalfibertracking");
//  right->AddView("org.mitk.views.partialvolumeanalysis");
//  right->AddView("org.mitk.views.ivim");
//  right->AddView("org.mitk.views.tractbasedspatialstatistics");
//  right->AddView("org.mitk.views.fibertracking");

  ////////////////////////////////////////
  // end internal views
  ////////////////////////////////////////

}
