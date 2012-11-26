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



#include "mitkStandaloneDataStorage.h"
#include "mitkDataNode.h"
#include "mitkEventInteractor.h"
#include "mitkVtkPropRenderer.h"
#include "mitkTestingMacros.h"
#include "mitkGlobalInteraction.h"


int mitkDispatcherTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("Dispatcher")

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // Here BindDispatcherInteractor and Dispatcher should be created automatically
  vtkRenderWindow* renWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New( "ContourRenderer",renWin, mitk::RenderingManager::GetInstance() );


  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  mitk::DataNode::Pointer dn = mitk::DataNode::New();

  mitk::EventInteractor::Pointer ei = mitk::EventInteractor::New();


  MITK_TEST_CONDITION_REQUIRED(
      renderer->GetDispatcher()->GetNumberOfInteractors() == 0
    , "Check Initialization of Dispatcher");

  ei->SetDataNode(dn);

  renderer->SetDataStorage(ds);
  ds->Add(dn);

  MITK_TEST_CONDITION_REQUIRED(
        renderer->GetDispatcher()->GetNumberOfInteractors() == 1
      , "Check that Interactor has been registered with Dispatcher");

  renWin->Delete();


  // always end with this!
  MITK_TEST_END()


}
