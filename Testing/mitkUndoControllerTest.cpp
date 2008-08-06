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

#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**
*  Simple example for a test for the (non-existent) class "UndoController".
*  
*  argc and argv are the command line parameters which were passed to 
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/
int mitkUndoControllerTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("UndoController")

  // let's create an object of our class  
  mitk::UndoController* myUndoController = new mitk::UndoController();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myUndoController != NULL, "Testing instantiation") 

  //check default model (verbose...)
  mitk::VerboseLimitedLinearUndo::Pointer standardModel = dynamic_cast<mitk::VerboseLimitedLinearUndo*>(myUndoController->GetCurrentUndoModel());
  MITK_TEST_CONDITION_REQUIRED(standardModel.IsNotNull(),"Testing if the standard undo model VerboseLimitedLinearUndo is returned") 

  //switch to limitedlinearundomodel
  myUndoController->AddUndoModel(mitk::UndoController::LIMITEDLINEARUNDO);
  mitk::LimitedLinearUndo::Pointer linearModel = dynamic_cast<mitk::LimitedLinearUndo*>(myUndoController->GetCurrentUndoModel());
  MITK_TEST_CONDITION_REQUIRED(linearModel.IsNotNull(),"Testing to add and then to read a LimitedLinearUndoModel") 

  //switching to verbose again
  myUndoController->SwitchUndoModel(mitk::UndoController::VERBOSE_LIMITEDLINEARUNDO);
  mitk::VerboseLimitedLinearUndo::Pointer anotherVerboseModelPointer = dynamic_cast<mitk::VerboseLimitedLinearUndo*>(myUndoController->GetCurrentUndoModel());
  MITK_TEST_CONDITION_REQUIRED(standardModel == anotherVerboseModelPointer,"Testing to switch back again and to be sure, that the poiinters are the same") 

  //removing verbose; model should be switch to limited
  myUndoController->RemoveUndoModel(mitk::UndoController::VERBOSE_LIMITEDLINEARUNDO);
  mitk::LimitedLinearUndo::Pointer anotherLinearModel = dynamic_cast<mitk::LimitedLinearUndo*>(myUndoController->GetCurrentUndoModel());
  MITK_TEST_CONDITION_REQUIRED(linearModel == anotherLinearModel,"Testing to remove the VerboseLimitedLinearUndoModel and to automatically switch to LimitedLinearUndo") 

  //switch to limitedlinearundomodel
  myUndoController->AddUndoModel(mitk::UndoController::VERBOSE_LIMITEDLINEARUNDO);
  mitk::VerboseLimitedLinearUndo::Pointer newVerboseModelPointer = dynamic_cast<mitk::VerboseLimitedLinearUndo*>(myUndoController->GetCurrentUndoModel());
  MITK_TEST_CONDITION_REQUIRED(newVerboseModelPointer != standardModel,"Testing to add verbose model and if the new model is equal to the deleted one. Should not be the case!") 

  //removing boith models 
  myUndoController->RemoveUndoModel(mitk::UndoController::VERBOSE_LIMITEDLINEARUNDO);
  //should not detele, to maintain an UndoController with at least one UndoModel
  myUndoController->RemoveUndoModel(mitk::UndoController::LIMITEDLINEARUNDO);

  mitk::LimitedLinearUndo::Pointer limited = dynamic_cast<mitk::LimitedLinearUndo*>(myUndoController->GetCurrentUndoModel());
  MITK_TEST_CONDITION_REQUIRED(limited.IsNotNull(),"Testing to erase all models. Should be impossible to maintain a working model in UndoController")

  delete myUndoController;

  // always end with this!
  MITK_TEST_END()
}

