/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  vtkRenderWindow *m_RenWin;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  
  mitk::StandaloneDataStorage::Pointer m_Ds;
  mitk::DataNode::Pointer m_Dn;

  mitk::DataNode::Pointer m_Dn2;
  mitk::DataInteractor::Pointer m_Ei;
  mitk::DataInteractor::Pointer m_Ei2;
public:

  /*
  * Tests the process of creating Interactors and assigning DataNodes to them.
  * Test checks if these Interactors are added to the Dispatcher under different conditions,
  * and in different call order.
  */

  void setUp()
  {
    // Here BindDispatcherInteractor and Dispatcher should be created automatically
    m_RenWin = vtkRenderWindow::New();
    m_Renderer = mitk::VtkPropRenderer::New("ContourRenderer", m_RenWin);
    m_Ds = mitk::StandaloneDataStorage::New();
    m_Dn = mitk::DataNode::New();
    m_Dn2 = mitk::DataNode::New();
    m_Ei = mitk::DataInteractor::New();
    m_Ei2 = mitk::DataInteractor::New();
    m_Renderer->SetDataStorage(m_Ds);
  }

  void tearDown()
  {
    m_RenWin->Delete();
    m_Renderer = nullptr;
    m_Ds = nullptr;
    m_Dn = nullptr;
    m_Dn2 = nullptr;
    m_Ei = nullptr;
    m_Ei2 = nullptr;
  }

  void DispatcherExists_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("01 Check Existence of Dispatcher.",
      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void AddInteractorConnectedToDataStorage_IncreaseNumberOfInteractors()
  {
    m_Ei->SetDataNode(m_Dn);
    m_Ds->Add(m_Dn);

    CPPUNIT_ASSERT_MESSAGE("02 Expected number of registered Interactors is 1",
                      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void AddDataNodeToInteractor_NoIncreasedNumberOfInteractors()
  {
    m_Ei->SetDataNode(m_Dn);
    m_Renderer->SetDataStorage(m_Ds);
    m_Ds->Add(m_Dn);
    // This _must not_ result in additionally registered interactors.
    m_Ei->SetDataNode(m_Dn);
    CPPUNIT_ASSERT_MESSAGE("03 Expected number of registered Interactors is 1",
                      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void AddInteractorNotConnectedToDataStorage_NoRegisteredInteractor()
  {
    // Switching the DataNode of an Interactor also must not result in extra registered Interactors in Dispatcher
    // since dn2 is not connected to DataStorage
    m_Ei->SetDataNode(m_Dn2);
    
    CPPUNIT_ASSERT_MESSAGE("04 Expected number of registered Interactors is 0",
                      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void ConnectInteractorToDataStorage_ReplaceInteractorEntry()
  {
    m_Ei->SetDataNode(m_Dn2);
    // DataNode Added to DataStorage, now Interactor entry in Dispatcher should be replaced,
    // hence we restore Interactor in the Dispatcher
    m_Ds->Add(m_Dn2);
    
    CPPUNIT_ASSERT_MESSAGE("05 Expected number of registered Interactors is 1",
                      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void NewDataNodeAndInteractor_IncreasedNumberOfInteractors()
  {
    m_Ei->SetDataNode(m_Dn2);
    m_Ds->Add(m_Dn2);
    m_Ds->Add(m_Dn);
    // New DataNode and new interactor, this should result in additional Interactor in the Dispatcher.
    m_Ei2->SetDataNode(m_Dn);

    CPPUNIT_ASSERT_MESSAGE("06 Exprected number of registered Interactors is 2",
                       m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 2);
  }

  void InteractorsPointToSameDataNode_DecreasedNumberOfInteractors()
  {
    m_Ds->Add(m_Dn2);
    m_Ds->Add(m_Dn);
    m_Ei->SetDataNode(m_Dn2);
    m_Ei2->SetDataNode(m_Dn);
    // Here ei and ei2 point to the same dn2; dn2 now only points to ei2, so ei is abandoned,
    // therefore ei1 is expected to be removed
    
    m_Ei2->SetDataNode(m_Dn2);
    CPPUNIT_ASSERT_MESSAGE("07 Expected number of registered Interactors is 1",
                      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
  }

  void SetDataNodeToNullptr_RemoveInteractor()
  {
    m_Ds->Add(m_Dn2);
    m_Ei2->SetDataNode(m_Dn2);
    // Setting DataNode in Interactor to nullptr, should remove Interactor from Dispatcher
    m_Ei2->SetDataNode(nullptr);
    CPPUNIT_ASSERT_MESSAGE("08 Expected number of registered Interactors is 0",
                    m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void RemoveDataNode_RemoveInteractor()
  {
    // Add DN again check if it is registered
    m_Ds->Add(m_Dn);
    m_Ei2->SetDataNode(m_Dn);
    
    CPPUNIT_ASSERT_MESSAGE("09 Expected number of registered Interactors is 1",
                      m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 1);
    
    // If DN is removed Interactors should be too
    m_Ds->Remove(m_Dn);
    CPPUNIT_ASSERT_MESSAGE("10 ExpectedNumber of registered Interactors is 0",
                     m_Renderer->GetDispatcher()->GetNumberOfInteractors() == 0);
  }

  void GetReferenceCountDataNode_Success()
  {
    m_Ds->Add(m_Dn);
    m_Ei2->SetDataNode(m_Dn);
    m_Ds->Remove(m_Dn);
    // after DN is removed from DS its reference count must be back to one
    CPPUNIT_ASSERT_MESSAGE("11 Expected number of references of DataNode is 1",
                                                  m_Dn->GetReferenceCount() == 1);
  }

  void GetReferenceCountInteractors_Success()
  {
    m_Ei->SetDataNode(m_Dn2);
    m_Ds->Add(m_Dn2);
    m_Ei2->SetDataNode(m_Dn2);
    CPPUNIT_ASSERT_MESSAGE("12 Expected number of references of Interactors is 1",
                                                     m_Ei->GetReferenceCount() == 1);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkDispatcher)
