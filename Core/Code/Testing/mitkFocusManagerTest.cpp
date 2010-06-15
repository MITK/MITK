/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 14:52:01 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17230 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkFocusManager.h"
#include "mitkTestingMacros.h"
#include "mitkVtkPropRenderer.h"
#include "mitkGlobalInteraction.h"

int mitkFocusManagerTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("FocusManager");

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  mitk::RenderingManager::Pointer rm = mitk::RenderingManager::GetInstance();

  //building up necessary objects
  vtkRenderWindow* renderWindow = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer element1 = mitk::VtkPropRenderer::New( "renderer1", renderWindow, rm );
  mitk::VtkPropRenderer::Pointer element2 = mitk::VtkPropRenderer::New( "renderer2", renderWindow, rm );
  mitk::VtkPropRenderer::Pointer element3 = mitk::VtkPropRenderer::New( "renderer3", renderWindow, rm );

  //the FocusManager itself
  mitk::FocusManager::Pointer focusManager = mitk::FocusManager::New();

  //testing
  MITK_TEST_CONDITION_REQUIRED(focusManager.IsNotNull(), "Testing Instatiation");
  
  MITK_TEST_CONDITION_REQUIRED(element1.IsNotNull(), "Testing Instatiation of an element");

  MITK_TEST_CONDITION_REQUIRED(focusManager->AddElement(element1), "Testing addition of an element");

  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element1, "Testing if the added element is focused on");

  MITK_TEST_CONDITION_REQUIRED(focusManager->RemoveElement(element1), "Testing removing of an element");
  
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == NULL, "Testing focused on an empty list");
  
  MITK_TEST_CONDITION_REQUIRED(focusManager->AddElement(element1), "Testing addition of an element; Elements in list: 1");
  MITK_TEST_CONDITION_REQUIRED(focusManager->AddElement(element2), "Testing addition of a second element; Elements in list: 1 2");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element1, "Testing if the added element still is element 1");
  MITK_TEST_CONDITION_REQUIRED(focusManager->AddElement(element3), "Testing addition of a third element; Elements in list: 1 2 3");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element1, "Testing if the added element still is element 1");

  MITK_TEST_CONDITION_REQUIRED(focusManager->SetFocused(element1), "Testing setting focused to element 1");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element1, "focus on element 1");
  MITK_TEST_CONDITION_REQUIRED(focusManager->SetFocused(element2), "Testing setting focused to element 2");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element2, "focus on element 2");
  MITK_TEST_CONDITION_REQUIRED(focusManager->SetFocused(element3), "Testing setting focused to element 3");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element3, "focus on element 3");

  MITK_TEST_CONDITION_REQUIRED(focusManager->RemoveElement(element1), "Testing removing first element; Elements in list: 2 3");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element2, "Testing if focused element is the one behind the deleted one: 2");
  MITK_TEST_CONDITION_REQUIRED(focusManager->AddElement(element1), "Testing addition of an element again; Elements in list: 2 3 1");
  MITK_TEST_CONDITION_REQUIRED(focusManager->RemoveElement(element3), "Testing removing element 3; Elements in list: 2 1");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element1, "Testing if focused element is the one behind the deleted one: 1");

  MITK_TEST_CONDITION_REQUIRED(focusManager->RemoveElement(element1), "Testing removing last element 1; Elements in list: 2 ");
  MITK_TEST_CONDITION_REQUIRED(focusManager->GetFocused() == element2, "Testing if focused element is 2");

  MITK_TEST_CONDITION_REQUIRED(!focusManager->RemoveElement(element1), "Testing removing same element");
  MITK_TEST_CONDITION_REQUIRED(focusManager->RemoveElement(element2), "Testing removing last element in list (2)");

  MITK_TEST_CONDITION_REQUIRED(!focusManager->RemoveElement(element3), "Testing removing from empty list ");
  MITK_TEST_CONDITION_REQUIRED(!focusManager->RemoveElement(element2), "Testing removing from empty list with different object");
  MITK_TEST_CONDITION_REQUIRED(!focusManager->RemoveElement(element1), "Testing removing from empty list with different object again");

  focusManager = NULL;
    
  //TODO: test IsLast() IsFirst() GetFirst() GetLast() GoToNext() GetIter() SetLoop(bool loop)

  //Delete renderWindo correctly
  renderWindow->Delete();

  MITK_TEST_END();
}