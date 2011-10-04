/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-05-03 12:55:29 +0200 (Mo, 03 Mai 2010) $
Version:   $Revision: 22655 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkNodeDepententPointSetInteractor.h"
#include "mitkPointSet.h"
#include "mitkPositionEvent.h"
#include "mitkVtkPropRenderer.h"
#include "mitkStateEvent.h"
#include "mitkInteractionConst.h"
#include "mitkGlobalInteraction.h"
#include "mitkPointOperation.h"
#include "mitkTestingMacros.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkDataNodeFactory.h"
#include "mitkStandardFileLocations.h"
#include "mitkNodePredicateDataType.h"

void SendPositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition, const mitk::Point3D& worldPosition)
{
  mitk::Event *posEvent = new mitk::PositionEvent(sender, type, button, buttonState, key, displPosition, worldPosition);
  mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(posEvent);
  delete posEvent;
}

//test related to tutorial Step5.cpp
int mitkNodeDependentPointSetInteractorTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("NodeDependentPointSetInteractor");

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // Create a DataStorage
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  MITK_TEST_CONDITION_REQUIRED(ds.IsNotNull(),"Instantiating DataStorage");

  //read two images and store to datastorage
  //these two images are used as node the interactors depend on. If the visibility property of one node if false, the 
  //associated interactor may not change the data
  mitk::DataNode::Pointer node1, node2;      
  mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
  
  MITK_TEST_CONDITION_REQUIRED(argc >= 3, "Test if a files to load has been specified");


  try
  {
    //file 1
    const std::string filename1 = argv[1];
    nodeReader->SetFileName(filename1);
    nodeReader->Update();
    node1 = nodeReader->GetOutput();
    ds->Add(node1);

    //file 2
    const std::string filename2 = argv[2];
    nodeReader->SetFileName(filename2);
    nodeReader->Update();
    node2 = nodeReader->GetOutput();
    ds->Add(node2);
  }
  catch(...) {
    MITK_TEST_FAILED_MSG(<< "Could not read file for testing");
    return NULL;
  }

  //check for the two images
  mitk::NodePredicateDataType::Pointer predicate(mitk::NodePredicateDataType::New("Image"));
  mitk::DataStorage::SetOfObjects::ConstPointer allImagesInDS = ds->GetSubset(predicate);
  MITK_TEST_CONDITION_REQUIRED(allImagesInDS->Size()==2,"load images to data storage");

  // Create PointSet and a node for it
  mitk::PointSet::Pointer pointSet1 = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode1 = mitk::DataNode::New();
  pointSetNode1->AddProperty( "unselectedcolor", mitk::ColorProperty::New(0.0f, 1.0f, 0.0f));
  pointSetNode1->SetData(pointSet1);
  mitk::NodeDepententPointSetInteractor::Pointer interactor1 = mitk::NodeDepententPointSetInteractor::New("pointsetinteractor", pointSetNode1, node1); 
  MITK_TEST_CONDITION_REQUIRED(interactor1.IsNotNull(),"Instanciating NodeDependentPointSetInteractor");
  // Add the node to the tree
  ds->Add(pointSetNode1);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor1);

  mitk::PointSet::Pointer pointSet2 = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode2 = mitk::DataNode::New();
  pointSetNode2->AddProperty( "unselectedcolor", mitk::ColorProperty::New(0.0f, 0.0f, 1.0f));
  pointSetNode2->SetData(pointSet2);
  mitk::NodeDepententPointSetInteractor::Pointer interactor2 = mitk::NodeDepententPointSetInteractor::New("pointsetinteractor", pointSetNode2, node2); 
  MITK_TEST_CONDITION_REQUIRED(interactor2.IsNotNull(),"Instanciating NodeDependentPointSetInteractor");
  // Add the node to the tree
  ds->Add(pointSetNode2);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor2);  

  //check for the two pointsets
  mitk::NodePredicateDataType::Pointer predicatePS(mitk::NodePredicateDataType::New("PointSet"));
  mitk::DataStorage::SetOfObjects::ConstPointer allImagesInDSPS = ds->GetSubset(predicatePS);
  MITK_TEST_CONDITION_REQUIRED(allImagesInDSPS->Size()==2,"create associated pointsets to data storage");

  //create two RenderWindows
  mitk::RenderingManager::Pointer myRenderingManager = mitk::RenderingManager::New();
  vtkRenderWindow* vtkRenWin1 = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer br1 = mitk::VtkPropRenderer::New("testingBR", vtkRenWin1, myRenderingManager);
  mitk::BaseRenderer::AddInstance(vtkRenWin1,br1);
  myRenderingManager->AddRenderWindow(vtkRenWin1);
  mitk::BaseRenderer::Pointer renderer1 = mitk::BaseRenderer::GetInstance(vtkRenWin1);

  vtkRenderWindow* vtkRenWin2 = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer br2 = mitk::VtkPropRenderer::New("testingBR", vtkRenWin2, myRenderingManager);
  mitk::BaseRenderer::AddInstance(vtkRenWin2,br2);
  myRenderingManager->AddRenderWindow(vtkRenWin2);
  mitk::BaseRenderer::Pointer renderer2 = mitk::BaseRenderer::GetInstance(vtkRenWin2);

  //set properties for renderWindow 1 and 2
  //1:
  node1->SetBoolProperty("visible", true, renderer1); 
  node2->SetBoolProperty("visible", false, renderer1); 
  //2:
  node1->SetBoolProperty("visible", false, renderer2); 
  node2->SetBoolProperty("visible", true, renderer2); 

  
  //***************************************************
  //now start to test if only an event send from renderwindow 1 can interact with interactor 1 and vice versa

  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==0,"Checking empty pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==0,"Checking empty pointset 2.");

  //sending an event to interactor1
  mitk::Point3D pos3D;
  mitk::Point2D pos2D;
  pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
  pos2D[0]= 100;  pos2D[0]= 200;

  //add to pointset 1
  SendPositionEvent(renderer1, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==1,"1 Checking addition of point to pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==0,"2 Checking empty pointset 2");

  //add to pointset 2
  SendPositionEvent(renderer2, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==1,"3 Checking untouched state of pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==1,"4 Checking addition of point to pointset 2");

  //add to pointset 2
  SendPositionEvent(renderer2, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==1,"5 Checking untouched state of pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==2,"6 Checking addition of point to pointset 2");

  //add to pointset 2
  SendPositionEvent(renderer2, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==1,"7 Checking untouched state of pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==3,"8 Checking addition of point to pointset 2");
  
  //add to pointset 1
  SendPositionEvent(renderer1, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==2,"9 Checking addition of point to pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==3,"10 Checking untouched state of pointset 2");

  //trying to delete points
  mitk::Event* delEvent1 = new mitk::Event(renderer1, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
  mitk::Event* delEvent2 = new mitk::Event(renderer2, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
  
  mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent2);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==2,"11 Checking untouched state of pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==2,"12 Checking detected point in pointset 2");

  mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent1);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==1,"11 Checking deleted point in pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==2,"12 Checking untouched state of pointset 2");
  
  mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent2);
  MITK_TEST_CONDITION_REQUIRED(pointSet1->GetPointSet()->GetNumberOfPoints()==1,"13 Checking untouched state of pointset 1");
  MITK_TEST_CONDITION_REQUIRED(pointSet2->GetPointSet()->GetNumberOfPoints()==1,"14 Checking detected point in pointset 2");

  mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor1);
  mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor2);
  delete delEvent1;
  delete delEvent2;
  
  myRenderingManager->RemoveRenderWindow(vtkRenWin1);
  myRenderingManager->RemoveRenderWindow(vtkRenWin2);
  vtkRenWin1->Delete();
  vtkRenWin2->Delete();

  //destroy RenderingManager
  myRenderingManager = NULL;


  MITK_TEST_END()
}
