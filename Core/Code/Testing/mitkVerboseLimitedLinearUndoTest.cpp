/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkOperation.h"
#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include "mitkInteractionConst.h"

#include "mitkTestingMacros.h"

#include <iostream>

int g_GlobalCounter = 0;

namespace mitk {

/**
* @brief Class to check that the destructor of object Operation is called and memory freed
**/
class TestOperation : public Operation
{
public:
  TestOperation(OperationType operationType)
    : Operation(operationType)
  { 
    g_GlobalCounter++; 
  };

  virtual ~TestOperation()
  {
    g_GlobalCounter--;
  };
};
}//namespace


/**
*  @brief Test of the LimitedLinearUndo object
*
*  This test was motivated by bug 3248 which had to check memory leakage
*  while using mitkOperations within the UndoMechanism. 
*  OperationObjects are added to the UndoController and stored within 
*  two lists (m_UndoList and m_RedoList) inside LimitedLinearUndo and 
*  derived from this VerboseLimitedLinearUndo. When using Undo during 
*  runtime operations are moved from UndoList to RedoList. In case of 
*  a new interaction, causing new operations to be stored in the UndoList
*  the RedoList needs to be cleared. For this, the operations and all 
*  connected objects need to be deleted and memory to be freed. 
*  And this what this test checks!
*  
*  argc and argv are the command line parameters which were passed to 
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/
int mitkVerboseLimitedLinearUndoTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("VerboseLimitedLinearUndo")

  // an UndoController for the management
  mitk::UndoController* myUndoController = new mitk::UndoController();

  //set model, even if it is verboseLimitedLinearUndo by default; this already is tested by UndoControllerTest!
  myUndoController->SwitchUndoModel(mitk::UndoController::VERBOSE_LIMITEDLINEARUNDO);

  for (int i = 0; i<2; i++)
  {
    mitk::TestOperation* doOp = new mitk::TestOperation(mitk::OpTEST);
    mitk::TestOperation *undoOp = new mitk::TestOperation(mitk::OpTEST);
    mitk::OperationEvent *operationEvent = new mitk::OperationEvent(NULL, doOp, undoOp, "Test");
    myUndoController->SetOperationEvent(operationEvent);
    //increase the ID to separate the operationEvents from each other. Otherwise they would be undone all together at once.
    mitk::OperationEvent::IncCurrObjectEventId();
    mitk::UndoStackItem::ExecuteIncrement();
  }

  //now 2 * 2 operation should have been instanciated
  MITK_TEST_CONDITION_REQUIRED(g_GlobalCounter == 4,"checking initialization of mitkOperation");

  //undo one operation; 1 operationEvent element in undo list, 1 in Redo list
  myUndoController->Undo(); 

  //sending two new OperationEvents: RedoList should be deleted and memory of operations freed
  for (int i = 0; i<2; i++)
  {
    mitk::TestOperation* doOp = new mitk::TestOperation(mitk::OpTEST);
    mitk::TestOperation *undoOp = new mitk::TestOperation(mitk::OpTEST);
    mitk::OperationEvent *operationEvent = new mitk::OperationEvent(NULL, doOp, undoOp, "Test");
    myUndoController->SetOperationEvent(operationEvent);
    //increase the ID to separate the operationEvents from each other. Otherwise they would be undone all together at once.
    mitk::OperationEvent::IncCurrObjectEventId();
    mitk::UndoStackItem::ExecuteIncrement();
  }
  
  //2 operations should have been deleted, 4 should have been added
  MITK_TEST_CONDITION_REQUIRED(g_GlobalCounter == 6,"checking adding of operations");
  
  //two operations to RedoList
  myUndoController->Undo(); 
  myUndoController->ClearRedoList();

  //one operationEvent containing 2 operations should have been deleted
  MITK_TEST_CONDITION_REQUIRED(g_GlobalCounter == 4,"checking deleting RedoList");

  //clear all
  myUndoController->Clear();
  MITK_TEST_CONDITION_REQUIRED(g_GlobalCounter == 0,"checking deleting all operations in UndoModel");

  //sending two new OperationEvents
  for (int i = 0; i<2; i++)
  {
    mitk::TestOperation* doOp = new mitk::TestOperation(mitk::OpTEST);
    mitk::TestOperation *undoOp = new mitk::TestOperation(mitk::OpTEST);
    mitk::OperationEvent *operationEvent = new mitk::OperationEvent(NULL, doOp, undoOp, "Test");
    myUndoController->SetOperationEvent(operationEvent);
    //increase the ID to separate the operationEvents from each other. Otherwise they would be undone all together at once.
    mitk::OperationEvent::IncCurrObjectEventId();
    mitk::UndoStackItem::ExecuteIncrement();
  }
  MITK_TEST_CONDITION_REQUIRED(g_GlobalCounter == 4,"checking added operations in UndoModel");

  delete myUndoController;

  //after deleting UndoController g_GlobalCounter will still be 4 because m_CurrentUndoModel inside myUndoModel is a static singleton
  MITK_TEST_CONDITION_REQUIRED(g_GlobalCounter == 4,"checking singleton UndoModel");

  // always end with this!
  MITK_TEST_END()
  //operations will be deleted after terminating the application
}

