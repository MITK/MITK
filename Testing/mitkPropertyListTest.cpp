/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include <iostream>

int mitkPropertyListTest(int /*argc*/, char* /*argv*/[])
{
  mitk::PropertyList::Pointer propList;
  std::cout << "Testing mitk::PropertyList::New(): ";
  propList = mitk::PropertyList::New();
  if (propList.IsNull()) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout << "[PASSED]" << std::endl;
  } 
  mitk::BoolProperty::Pointer boolProp = new mitk::BoolProperty(false);
  mitk::BoolProperty::Pointer boolProp2 = new mitk::BoolProperty(false);
  std::cout << "Testing BoolProperty ==: ";
  if (! (*boolProp2 == *boolProp) ) {
    
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    } 
  std::cout << "[PASSED]" << std::endl;
  unsigned long tBefore,tAfter; 
  
  std::cout << "Testing SetProperty() with new key value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",boolProp); 
  tAfter = propList->GetMTime();
  if (! ( tAfter > tBefore) ) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  std::cout << "Testing SetProperty() with changed property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",new mitk::BoolProperty(true)); 
  tAfter = propList->GetMTime();
  if (!  (tAfter > tBefore) ) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  std::cout << "Testing SetProperty() with unchanged property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",new mitk::BoolProperty(true)); 
  tAfter = propList->GetMTime();
  if ( tBefore != tAfter ) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  std::cout << "Testing MTime correctness when changing property value: ";
  boolProp = new mitk::BoolProperty(true);
  propList->ReplaceProperty("test",boolProp); 
  tBefore = propList->GetMTime();
  boolProp->SetValue(true);
  tAfter = propList->GetMTime();
  boolProp->SetValue(false);
  unsigned long tAfterAll = propList->GetMTime();
  
  if (tBefore != tAfter || tAfterAll <= tAfter) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
 
  std::cout << "Testing MTime correctness when calling SetProperty twice: ";
  boolProp = new mitk::BoolProperty(true);
  propList->SetProperty("test",boolProp); 
  tBefore = propList->GetMTime();
  propList->SetProperty("test",boolProp); 
  tAfter = propList->GetMTime();
  
  if (tBefore != tAfter) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
 

  std::cout << "Testing if existing properties survive SetProperty: ";
  propList->SetProperty("test",boolProp);
  mitk::BaseProperty* bpBefore = propList->GetProperty("test");
  propList->SetProperty("test",boolProp2);
  mitk::BaseProperty* bpAfter = propList->GetProperty("test");

  if (bpBefore != bpAfter || bpAfter == NULL) {
    std::cout << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing if existing properties survive ReplaceProperty: ";
  propList->SetProperty("test",boolProp);
  bpBefore = propList->GetProperty("test");
  propList->ReplaceProperty("test",boolProp2);
  bpAfter = propList->GetProperty("test");

  if (bpBefore == bpAfter || bpAfter == NULL) {
    std::cout << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
    

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
