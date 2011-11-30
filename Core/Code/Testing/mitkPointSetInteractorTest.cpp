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


#include "mitkPointSetInteractor.h"
#include "mitkPointSet.h"
#include "mitkPositionEvent.h"
#include "mitkVtkPropRenderer.h"
#include "mitkStateEvent.h"
#include "mitkInteractionConst.h"
#include "mitkGlobalInteraction.h"
#include "mitkPointOperation.h"
#include "mitkTestingMacros.h"

#include <iostream>

const char* POINTSETINTERACTORNAME = "pointsetinteractor";
const char* ONLYMOVEPOINTSETINTERACTORNAME = "onlymovepointsetinteractor";
const char* SEEDPOINTSETINTERACTORNAME = "seedpointsetinteractor";
const char* SINGLEPOINTWITHOUTSHIFTCLICKNAME = "singlepointinteractorwithoutshiftclick";

/**
*@brief method to send specified events to EventMapper
**/
class mitkPointSetInteractorTestClass { 
public:

  void TestPointSetInteractor(const char* name, mitk::DataNode* node, mitk::BaseRenderer* sender, int numberOfPointsAllowed)
  {  
    mitk::PointSetInteractor::Pointer interactor = mitk::PointSetInteractor::New(name, node, numberOfPointsAllowed); 
    MITK_TEST_CONDITION_REQUIRED(interactor.IsNotNull(),"Testing to initialize PointSetInteractor")
      std::cout<<"The pattern of the interactor is called: "<<interactor->GetType()<<std::endl;
    MITK_TEST_CONDITION_REQUIRED(interactor->GetType() == name,"testing pattern name of interactor");

    //should not be null
    MITK_TEST_CONDITION_REQUIRED(node != NULL, "error in test! Node == NULL");
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(), "error in test! PointSet not set");

    //sending an event now shouln't lead to an addition of a point because interactor is not yet connected to globalinteraction

    mitk::Point3D pos3D;
    mitk::Point2D pos2D;
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 100;  pos2D[0]= 200;
    
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking unconnected interactor.");

    //activate interaction
    mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

    //now one point should be added going from state 1 over state 3 and 40 to state 2 (space left)
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking connected interactor by adding a point.");

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing right addition of point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point is selected.");

    //delesecting point; going from state 2 over state 10 to state 2
    pos3D.Fill(-100.0);
    pos2D.Fill(200.0);
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) == false,"Testing deselection of a point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetNumberOfSelected() == 0,"No selected points.");

    //trying to delete a deselected point so going from state 2 over 30 to 1 
    mitk::Event* delEvent = new mitk::Event(sender, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking that no unselected point can be deleted.");
    //not deleting delEvent, because if will be used later on

    //picking point
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 100;  pos2D[0]= 200;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point is picked.");

    //deleting selected point
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if the selected point could be delected.");

    //adding two points and checking selection
    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking adding point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point1 is selected.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing addition of point.");

    pos3D[0]= 111.0;  pos3D[1]= 222.0;  pos3D[2]= 333.0;
    pos2D[0]= 111;  pos2D[0]= 222;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==2,"Checking adding second point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ==false,"Testing if point1 is deselected.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(1) == pos3D,"Testing addition of point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(1) ,"Testing if point2 is selected.");


    //selecting the first point and deleting it
    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point1 is picked.");

    //sending delete-event
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking if the picked point1 could be delected.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(1) ,"Testing if point2 is now selected.");
    //sending delete-event again
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if point2 could be delected.");

    //adding more than three points and see if only three can be added
    pos3D[0]= 1.0;  pos3D[1]= 2.0;  pos3D[2]= 3.0;
    pos2D[0]= 1;  pos2D[0]= 2;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 111.0;  pos3D[1]= 222.0;  pos3D[2]= 333.0;
    pos2D[0]= 111;  pos2D[0]= 222;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 1111.0;  pos3D[1]= 2222.0;  pos3D[2]= 3333.0;
    pos2D[0]= 12;  pos2D[0]= 21;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    if (numberOfPointsAllowed>=0) //not number of points set to unlimited
    {
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==(unsigned long)numberOfPointsAllowed,"Checking if only the amount of defined points could be added.");
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(2) ,"Testing if the last point added is selected.");
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(2) != pos3D,"Testing that the last addition didn't work.");
    }

    //testing whether a point can be added to an already filled point set after deleting an unselected point
    if (numberOfPointsAllowed>=0) //not number of points set to unlimited
    {
      //delesecting point; going from state 2 over state 10 to state 2
      pos3D.Fill(-100.0);
      pos2D.Fill(200.0);
      this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) == false,"Testing deselection of a point.");
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetNumberOfSelected() == 0,"No selected points.");
      //trying to delete a deselected point so going from state 2 over 30 to 1
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      unsigned long nPts = pointSet->GetPointSet()->GetNumberOfPoints();
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==(unsigned long)numberOfPointsAllowed,"Checking that no unselected point can be deleted.");
      //trying to add point to already filled point set
      this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==(unsigned long)numberOfPointsAllowed,"Checking that no point can be added after hitting DEL on no selection.");
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(2) ,"Testing if the last point added is selected.");
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(2) != pos3D,"Testing that the last addition didn't work.");
    }

    //selecting the first point and moving it to 0,0,0
    pos3D[0]= 1.0;  pos3D[1]= 2.0;  pos3D[2]= 3.0;
    pos2D[0]= 1;  pos2D[0]= 2;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point1 is selected.");
    //sending the same event to hold the point for movement
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);

    //slowly move to 0,0,0
    pos3D[0]= 0.5;  pos3D[1]= 1.0;  pos3D[2]= 1.5;
    pos2D[0]= 0;  pos2D[0]= 1;
    this->SendPositionEvent(sender, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 0.0;  pos3D[1]= 0.0;  pos3D[2]= 0.0;
    pos2D[0]= 0;  pos2D[0]= 0;
    this->SendPositionEvent(sender, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    //release event
    this->SendPositionEvent(sender, mitk::Type_MouseButtonRelease, mitk::BS_LeftButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing movement of point.");

    if (numberOfPointsAllowed>=0) //not number of points set to unlimited
    {
      //deleting all three points now
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==2,"Checking if point3 could be delected.");
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking if point2 could be delected.");
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if point1 could be delected.");
    }
    else
    {
      //deleting all four points now
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==3,"Checking if point4 could be delected.");
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==2,"Checking if point3 could be delected.");
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking if point2 could be delected.");
      mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
      MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if point1 could be delected.");
    }

    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
    delete delEvent;
  }

  void TestOnlyMovePointSetInteractor(const char* name, mitk::DataNode* node, mitk::BaseRenderer* sender, int numberOfPointsAllowed)
  {  
    mitk::PointSetInteractor::Pointer interactor = mitk::PointSetInteractor::New(name, node, numberOfPointsAllowed); 
    MITK_TEST_CONDITION_REQUIRED(interactor.IsNotNull(),"Testing to initialize PointSetInteractor")
      std::cout<<"The pattern of the interactor is called: "<<interactor->GetType()<<std::endl;
    MITK_TEST_CONDITION_REQUIRED(interactor->GetType() == name,"testing pattern name of interactor");

    MITK_TEST_CONDITION_REQUIRED(node != NULL, "error in test! Node == NULL");
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(), "error in test! PointSet not set");

    //sending an event now shouln't lead to an addition of a point because interactor is not connected to globalinteraction
    mitk::Point3D pos3D;
    mitk::Point2D pos2D;
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 10;  pos2D[0]= 20;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking unconnected interactor.");

    //activate interaction
    mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

    //sending event shouldn't lead to an addition of a point, because statemachine cannot handle this!
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Check to add a point.");

    //manually adding two points to a new pointset and setting it to node
    mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();
    //pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    //pos2D[0]= 10;  pos2D[0]= 20;
    int timestep = 0;
    int index = 0;
    mitk::PointOperation* doOp = new mitk::PointOperation( mitk::OpINSERT, timestep, pos3D, index);
    newPointSet->ExecuteOperation(doOp);
    //undo is enabled on default, so no need to delete doOp
    
    pos3D[0]= 100.0;  pos3D[1]= 200.0;  pos3D[2]= 300.0;
    pos2D[0]= 100;  pos2D[0]= 200;
    index = 1;
    mitk::PointOperation* secondDoOp = new mitk::PointOperation( mitk::OpINSERT, timestep, pos3D, index);
    newPointSet->ExecuteOperation(secondDoOp);

    //setting it to node
    node->SetData(newPointSet);
    pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(), "error in test! new PointSet not set");

    //checking if data has two points now
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints() == 2,"Testing new data.");

    //all points should be deselected but standard addition of a point is selected by default so all are selected. To be solved later
    //MITK_TEST_CONDITION_REQUIRED(pointSet->GetNumberOfSelected() == 2,"No selected points.");

    //trying to delete a selected point
    mitk::Event* delEvent = new mitk::Event(sender, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==2,"Checking that no point can be deleted.");
    delete delEvent;

    //delesecting point
    pos3D.Fill(-100.0);
    pos2D.Fill(200.0);
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(1) == false,"Testing deselection of a point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetNumberOfSelected() == 0,"No selected points.");

    //picking point
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 10;  pos2D[0]= 20;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point is picked.");

    //sending the same event to hold the point for movement
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);

    //slowly move to 0,0,0
    pos3D[0]= 0.5;  pos3D[1]= 1.0;  pos3D[2]= 1.5;
    pos2D[0]= 0;  pos2D[0]= 1;
    this->SendPositionEvent(sender, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 0.0;  pos3D[1]= 0.0;  pos3D[2]= 0.0;
    pos2D[0]= 0;  pos2D[0]= 0;
    this->SendPositionEvent(sender, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    //release event
    this->SendPositionEvent(sender, mitk::Type_MouseButtonRelease, mitk::BS_LeftButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing movement of point.");
    
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
  }

  
  void TestSeedPointSetInteractor(const char* name, mitk::DataNode* node, mitk::BaseRenderer* sender)
  {  
    mitk::PointSetInteractor::Pointer interactor = mitk::PointSetInteractor::New(name, node); 
    MITK_TEST_CONDITION_REQUIRED(interactor.IsNotNull(),"Testing to initialize PointSetInteractor")
      std::cout<<"The pattern of the interactor is called: "<<interactor->GetType()<<std::endl;
    MITK_TEST_CONDITION_REQUIRED(interactor->GetType() == name,"testing pattern name of interactor");

    //should not be null
    MITK_TEST_CONDITION_REQUIRED(node != NULL, "error in test! Node == NULL");
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(), "error in test! PointSet not set");

    //sending an event now shouln't lead to an addition of a point because interactor is not yet connected to globalinteraction

    mitk::Point3D pos3D;
    mitk::Point2D pos2D;
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 10;  pos2D[0]= 20;
    
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking unconnected interactor.");

    //activate interaction
    mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

    //now one point should be added going from state 1 over state 2 to state 10
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking connected interactor by adding a point.");

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing right addition of point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point is selected.");

    //delesecting point; going from state 10 over state 12 to state 10
    pos3D.Fill(-100.0);
    pos2D.Fill(200.0);
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) == false,"Testing deselection of a point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetNumberOfSelected() == 0,"No selected points.");

    //trying to delete a deselected point so going from state 10 over 15 to 10 
    mitk::Event* delEvent = new mitk::Event(sender, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking that no unselected point can be deleted.");
    //not deleting delEvent, because if will be used later on

    //picking point
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 10;  pos2D[0]= 20;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing picking point.");

    //deleting selected point
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if the selected point could be delected.");

    //adding two points and checking that only the last one remains in point set 
    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking adding point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point1 is selected.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing addition of point.");

    pos3D[0]= 111.0;  pos3D[1]= 222.0;  pos3D[2]= 333.0;
    pos2D[0]= 111;  pos2D[0]= 222;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking that only one point remains in pointset.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing addition of point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ==true,"Testing if point is selected.");

    //sending delete-event
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if the point could be delected.");
    //sending delete-event again
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking Sending Delete again.");

    //adding three points and see if only the third remains
    pos3D[0]= 1.0;  pos3D[1]= 2.0;  pos3D[2]= 3.0;
    pos2D[0]= 1;  pos2D[0]= 2;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 111.0;  pos3D[1]= 222.0;  pos3D[2]= 333.0;
    pos2D[0]= 111;  pos2D[0]= 222;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_ShiftButton, mitk::Key_none, pos2D, pos3D);

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking if only one point could be added.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing if the last point was inserted.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if the last point added is selected.");

    //sending the picking event to hold the point for movement
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);

    //slowly move to 0,0,0
    pos3D[0]= 0.5;  pos3D[1]= 1.0;  pos3D[2]= 1.5;
    pos2D[0]= 0;  pos2D[0]= 1;
    this->SendPositionEvent(sender, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 0.0;  pos3D[1]= 0.0;  pos3D[2]= 0.0;
    pos2D[0]= 0;  pos2D[0]= 0;
    this->SendPositionEvent(sender, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    //release event
    this->SendPositionEvent(sender, mitk::Type_MouseButtonRelease, mitk::BS_LeftButton, mitk::BS_LeftButton, mitk::Key_none, pos2D, pos3D);

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing movement of point.");

    //not deleting the points and end of test
    
    //removing interactor from GlobalInteraction
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
    delete delEvent;
  }

  
  void TestSinglePointSetInteractorWithoutShiftClick(const char* name, mitk::DataNode* node, mitk::BaseRenderer* sender)
  {  
    mitk::PointSetInteractor::Pointer interactor = mitk::PointSetInteractor::New(name, node); 
    MITK_TEST_CONDITION_REQUIRED(interactor.IsNotNull(),"Testing to initialize PointSetInteractor")
      std::cout<<"The pattern of the interactor is called: "<<interactor->GetType()<<std::endl;
    MITK_TEST_CONDITION_REQUIRED(interactor->GetType() == name,"testing pattern name of interactor");

    //should not be null
    MITK_TEST_CONDITION_REQUIRED(node != NULL, "error in test! Node == NULL");
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    MITK_TEST_CONDITION_REQUIRED(pointSet.IsNotNull(), "error in test! PointSet not set");

    //sending an event now shouln't lead to an addition of a point because interactor is not yet connected to globalinteraction

    mitk::Point3D pos3D;
    mitk::Point2D pos2D;
    pos3D[0]= 10.0;  pos3D[1]= 20.0;  pos3D[2]= 30.0;
    pos2D[0]= 10;  pos2D[0]= 20;
    
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking unconnected interactor.");

    //activate interaction
    mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

    //now one point should be added going from state 1 to state 2
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking connected interactor by adding a point.");

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing right addition of point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point is selected.");

    //trying to delete point 
    mitk::Event* delEvent = new mitk::Event(sender, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking deleting point.");

    //adding two points and checking that only the last one remains in point set 
    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking adding point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if point1 is selected.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing addition of point.");

    pos3D[0]= 111.0;  pos3D[1]= 222.0;  pos3D[2]= 333.0;
    pos2D[0]= 111;  pos2D[0]= 222;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking that only one point remains in pointset.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing addition of point.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ==true,"Testing if point is selected.");

    //sending delete-event
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if the point could be delected.");
    //sending delete-event again
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(delEvent);
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking Sending Delete again.");

    //adding three points and see if only the third remains
    pos3D[0]= 1.0;  pos3D[1]= 2.0;  pos3D[2]= 3.0;
    pos2D[0]= 1;  pos2D[0]= 2;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 11.0;  pos3D[1]= 22.0;  pos3D[2]= 33.0;
    pos2D[0]= 11;  pos2D[0]= 22;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);

    pos3D[0]= 111.0;  pos3D[1]= 222.0;  pos3D[2]= 333.0;
    pos2D[0]= 111;  pos2D[0]= 222;
    this->SendPositionEvent(sender, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none, pos2D, pos3D);

    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==1,"Checking if only one point could be added.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetPoint(0) == pos3D,"Testing if the last point was inserted.");
    MITK_TEST_CONDITION_REQUIRED(pointSet->GetSelectInfo(0) ,"Testing if the last point added is selected.");

    //removing interactor from GlobalInteraction
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
    delete delEvent;
  }

private:

  void SendPositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition, const mitk::Point3D& worldPosition)
  {
    mitk::Event *posEvent = new mitk::PositionEvent(sender, type, button, buttonState, key, displPosition, worldPosition);
    mitk::GlobalInteraction::GetInstance()->GetEventMapper()->MapEvent(posEvent);
    delete posEvent;
  }

}; //mitkPointSetInteractorTestClass



int mitkPointSetInteractorTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PointSetInteractor")

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  //create the corresponding data
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataNode::Pointer node = mitk::DataNode::New();

  //we need a baserenderer (VtkPropRenderer) to be able to let PointSetInteractor::CanHandleEvent() proccess
  //and for this we need a RenderWindow and a RenderingManager
  
  mitk::RenderingManager::Pointer myRenderingManager = mitk::RenderingManager::New();
  myRenderingManager->SetGlobalInteraction(mitk::GlobalInteraction::GetInstance());
  vtkRenderWindow* vtkRenWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer sender = mitk::VtkPropRenderer::New("testingBR", vtkRenWin, myRenderingManager);
  vtkRenWin->Delete();

  //hook everything up into a dataNode. (node doesn't here have to be in DataStorage)
  node->SetData(pointSet);

  //pointset should be empty  
  MITK_TEST_CONDITION_REQUIRED(pointSet->GetPointSet()->GetNumberOfPoints()==0,"Checking if pointset is empty.");

  //instance of testclass
  mitkPointSetInteractorTestClass* test = new mitkPointSetInteractorTestClass();
  
  //test setup regular pointsetinteractor
  MITK_TEST_OUTPUT(<<"--------Testing "<<POINTSETINTERACTORNAME<<" with max 3 points--------");
  test->TestPointSetInteractor(POINTSETINTERACTORNAME, node, sender, 3);
  
  pointSet = mitk::PointSet::New();
  node = mitk::DataNode::New();
  node->SetData(pointSet);

  //test setup with unlimited numbers of points supported
  MITK_TEST_OUTPUT(<<"--------Testing "<<POINTSETINTERACTORNAME<<" with unlimited points --------");
  test->TestPointSetInteractor(POINTSETINTERACTORNAME, node, sender, -1);

  pointSet = mitk::PointSet::New();
  node = mitk::DataNode::New();
  node->SetData(pointSet);

  //test setup only move pointsetinteractor
  MITK_TEST_OUTPUT(<<"--------Testing "<<ONLYMOVEPOINTSETINTERACTORNAME<<" with unlimited points --------");
  test->TestOnlyMovePointSetInteractor(ONLYMOVEPOINTSETINTERACTORNAME, node, sender, -1);

  pointSet = mitk::PointSet::New();
  node = mitk::DataNode::New();
  node->SetData(pointSet);

  MITK_TEST_OUTPUT(<<"--------Testing "<<SEEDPOINTSETINTERACTORNAME<<" with one point --------");
  test->TestSeedPointSetInteractor(SEEDPOINTSETINTERACTORNAME, node, sender);

  pointSet = mitk::PointSet::New();
  node = mitk::DataNode::New();
  node->SetData(pointSet);
  
  MITK_TEST_OUTPUT(<<"--------Testing "<<SINGLEPOINTWITHOUTSHIFTCLICKNAME<<" with one point --------");
  test->TestSinglePointSetInteractorWithoutShiftClick(SINGLEPOINTWITHOUTSHIFTCLICKNAME, node, sender);

  MITK_TEST_END()
}
