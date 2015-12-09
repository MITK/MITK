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
#include "mitkDataInteractor.h"
#include "mitkVtkPropRenderer.h"
#include "mitkBaseRenderer.h"
#include "mitkTestingMacros.h"
#include "itkLightObject.h"
#include "mitkDispatcher.h"

int mitkDispatcherTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("Dispatcher")

  /*
   * Tests the process of creating Interactors and assigning DataNodes to them.
   * Test checks if these Interactors are added to the Dispatcher under different conditions,
   * and in different call order.
   */

      // Here BindDispatcherInteractor and Dispatcher should be created automatically
      vtkRenderWindow* renWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New( "ContourRenderer",renWin, mitk::RenderingManager::GetInstance() );

  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  mitk::DataNode::Pointer dn;

  dn = mitk::DataNode::New();
  mitk::DataNode::Pointer dn2 = mitk::DataNode::New();
  mitk::DataInteractor::Pointer ei = mitk::DataInteractor::New();
  mitk::DataInteractor::Pointer ei2 = mitk::DataInteractor::New();


  MITK_TEST_CONDITION_REQUIRED(
        renderer->GetDispatcher()->GetNumberOfInteractors() == 0
        , "01 Check Existence of Dispatcher." );

  ei->SetDataNode(dn);
  renderer->SetDataStorage(ds);
  ds->Add(dn);

  int num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 1
        , "02 Number of registered Interactors " << num << " , expected 1" );

  // This _must not_ result in additionally registered interactors.
  ei->SetDataNode(dn);
  ei->SetDataNode(dn);

  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 1
        , "03 Number of registered Interactors " << num << " , expected 1" );

  // Switching the DataNode of an Interactor also must not result in extra registered Interactors in Dispatcher
  // since dn2 is not connected to DataStorage
  // ei will be dropped from dispatcher
  ei->SetDataNode(dn2);

  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 0
        , "04 Number of registered Interactors " << num << " , expected 0" );

  // DataNode Added to DataStorage, now Interactor entry in Dispatcher should be replaced,
  // hence we restore Interactor in the Dispatcher
  ds->Add(dn2);

  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 1
        , "05 Number of registered Interactors " << num << " , expected 1" );

  // New DataNode and new interactor, this should result in additional Interactor in the Dispatcher.

  ei2->SetDataNode(dn);

  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 2
        , "06 Number of registered Interactors " << num << " , expected 2" );

  // Here ei and ei2 point to the same dn2; dn2 now only points to ei2, so ei is abandoned,
  // therefore ei1 is expected to be removed

  ei2->SetDataNode(dn2);
  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 1
        , "07 Number of registered Interactors " << num << " , expected 1" );

  // Setting DataNode in Interactor to NULL, should remove Interactor from Dispatcher
  ei2->SetDataNode(NULL);
  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 0
        , "08 Number of registered Interactors " << num << " , expected 0" );


  // Add DN again check if it is registered

  ei2->SetDataNode(dn);

  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 1
        , "09 Number of registered Interactors " << num << " , expected 1" );


  // If DN is removed Interactors should be too
  ds->Remove(dn);
  num = renderer->GetDispatcher()->GetNumberOfInteractors();
  MITK_TEST_CONDITION_REQUIRED(
        num == 0
        , "10 Number of registered Interactors " << num << " , expected 0" );


  // after DN is removed from DS its reference count must be back to one

  MITK_TEST_CONDITION_REQUIRED(
        dn->GetReferenceCount() == 1
        , "10 Number of references of DataNode " << num << " , expected 1" );


  MITK_TEST_CONDITION_REQUIRED(
        ei->GetReferenceCount() == 1
        , "11 Number of references of Interactors " << num << " , expected 1" );

  renWin->Delete();
  // always end with this!
  MITK_TEST_END()

}
