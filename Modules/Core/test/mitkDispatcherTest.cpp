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

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// MITK includes
#include <mitkCoreServices.h>
#include "mitkBaseRenderer.h"
#include "mitkDataInteractor.h"
#include "mitkDataNode.h"
#include "mitkDispatcher.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkVtkPropRenderer.h"
// ITK includes
#include "itkLightObject.h"

class mitkDispatcherTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDispatcherTestSuite);
  MITK_TEST(DispatcherExists_Success);
  MITK_TEST(AddInteractorConnectedToDataStorage_IncreaseNumberOfInteractors);
  MITK_TEST(AddDataNodeToInteractor_NoIncreasedNumberOfInteractors);
  MITK_TEST(AddInteractorNotConnectedToDataStorage_NoRegisteredInteractor);
  MITK_TEST(ConnectInteractorToDataStorage_ReplaceInteractorEntry);
  MITK_TEST(NewDataNodeAndInteractor_IncreasedNumberOfInteractors);
  MITK_TEST(InteractorsPointToSameDataNode_DecreasedNumberOfInteractors);
  MITK_TEST(SetDataNodeToNullptr_RemoveInteractor);
  MITK_TEST(RemoveDataNode_RemoveInteractor);
  MITK_TEST(GetReferenceCountDataNode_Success);
  MITK_TEST(GetReferenceCountInteractors_Success);
  CPPUNIT_TEST_SUITE_END();

private:

  vtkRenderWindow *renWin;
  mitk::VtkPropRenderer::Pointer renderer;
  
  mitk::StandaloneDataStorage::Pointer ds;
  mitk::DataNode::Pointer dn;

  mitk::DataNode::Pointer dn2;
  mitk::DataInteractor::Pointer ei;
  mitk::DataInteractor::Pointer ei2;
public:

  /*
  * Tests the process of creating Interactors and assigning DataNodes to them.
  * Test checks if these Interactors are added to the Dispatcher under different conditions,
  * and in different call order.
  */

  void setUp()
  {
    // Here BindDispatcherInteractor and Dispatcher should be created automatically
    renWin = vtkRenderWindow::New();
    renderer = mitk::VtkPropRenderer::New("ContourRenderer", renWin, mitk::RenderingManager::GetInstance());
    ds = mitk::StandaloneDataStorage::New();
    dn = mitk::DataNode::New();
    dn2 = mitk::DataNode::New();
    ei = mitk::DataInteractor::New();
    ei2 = mitk::DataInteractor::New();
    renderer->SetDataStorage(ds);
  }

  void tearDown()
  {
    renWin->Delete();
    renderer = nullptr;
    ds = nullptr;
    dn = nullptr;
    dn2 = nullptr;
    ei = nullptr;
    ei2 = nullptr;
  }

  void DispatcherExists_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("01 Check Existence of Dispatcher.",
      renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void AddInteractorConnectedToDataStorage_IncreaseNumberOfInteractors()
  {
    ei->SetDataNode(dn);
    ds->Add(dn);

    CPPUNIT_ASSERT_MESSAGE("02 Expected number of registered Interactors is 1",
                      renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void AddDataNodeToInteractor_NoIncreasedNumberOfInteractors()
  {
    ei->SetDataNode(dn);
    renderer->SetDataStorage(ds);
    ds->Add(dn);
    // This _must not_ result in additionally registered interactors.
    ei->SetDataNode(dn);
    CPPUNIT_ASSERT_MESSAGE("03 Expected number of registered Interactors is 1",
                      renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void AddInteractorNotConnectedToDataStorage_NoRegisteredInteractor()
  {
    // Switching the DataNode of an Interactor also must not result in extra registered Interactors in Dispatcher
    // since dn2 is not connected to DataStorage
    ei->SetDataNode(dn2);
    
    CPPUNIT_ASSERT_MESSAGE("04 Expected number of registered Interactors is 0",
                      renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void ConnectInteractorToDataStorage_ReplaceInteractorEntry()
  {
    ei->SetDataNode(dn2);
    // DataNode Added to DataStorage, now Interactor entry in Dispatcher should be replaced,
    // hence we restore Interactor in the Dispatcher
    ds->Add(dn2);
    
    CPPUNIT_ASSERT_MESSAGE("05 Expected number of registered Interactors is 1",
                      renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void NewDataNodeAndInteractor_IncreasedNumberOfInteractors()
  {
    ei->SetDataNode(dn2);
    ds->Add(dn2);
    ds->Add(dn);
    // New DataNode and new interactor, this should result in additional Interactor in the Dispatcher.
    ei2->SetDataNode(dn);

    CPPUNIT_ASSERT_MESSAGE("06 Exprected number of registered Interactors is 2",
                       renderer->GetDispatcher()->GetNumberOfInteractors() == 2);
  }

  void InteractorsPointToSameDataNode_DecreasedNumberOfInteractors()
  {
    ds->Add(dn2);
    ds->Add(dn);
    ei->SetDataNode(dn2);
    ei2->SetDataNode(dn);
    // Here ei and ei2 point to the same dn2; dn2 now only points to ei2, so ei is abandoned,
    // therefore ei1 is expected to be removed
    
    ei2->SetDataNode(dn2);
    CPPUNIT_ASSERT_MESSAGE("07 Expected number of registered Interactors is 1",
                      renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void SetDataNodeToNullptr_RemoveInteractor()
  {
    ds->Add(dn2);
    ei2->SetDataNode(dn2);
    // Setting DataNode in Interactor to nullptr, should remove Interactor from Dispatcher
    ei2->SetDataNode(nullptr);
    CPPUNIT_ASSERT_MESSAGE("08 Expected number of registered Interactors is 0",
                    renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void RemoveDataNode_RemoveInteractor()
  {
    // Add DN again check if it is registered
    ds->Add(dn);
    ei2->SetDataNode(dn);
    
    CPPUNIT_ASSERT_MESSAGE("09 Expected number of registered Interactors is 1",
                      renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
    
    // If DN is removed Interactors should be too
    ds->Remove(dn);
    CPPUNIT_ASSERT_MESSAGE("10 ExpectedNumber of registered Interactors is 0",
                     renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void GetReferenceCountDataNode_Success()
  {
    ds->Add(dn);
    ei2->SetDataNode(dn);
    ds->Remove(dn);
    // after DN is removed from DS its reference count must be back to one
    CPPUNIT_ASSERT_MESSAGE("11 Expected number of references of DataNode is 1",
                                                  dn->GetReferenceCount() == 1);
  }

  void GetReferenceCountInteractors_Success()
  {
    ei->SetDataNode(dn2);
    ds->Add(dn2);
    ei2->SetDataNode(dn2);
    CPPUNIT_ASSERT_MESSAGE("12 Expected number of references of Interactors is 1",
                                                     ei->GetReferenceCount() == 1);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkDispatcher)
