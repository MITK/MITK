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

#include "mitkGenericProperty.h"

#include <iostream>

// call with testValue1 != testValue2 
template <typename T>
int TestGenericPropertyForDataType(T testValue1, T testValue2, std::string testValue1AsString, std::string testValue2AsString)
{
  std::cout << "Testing mitk::GenericProperty<>(: " << testValue1 << ", " << testValue2 << ") ";
  
  typename mitk::GenericProperty<T>::Pointer prop = new mitk::GenericProperty<T>();
  typename mitk::GenericProperty<T>::Pointer prop2 = new mitk::GenericProperty<T>(testValue1);
  typename mitk::GenericProperty<T>::Pointer prop3 = new mitk::GenericProperty<T>(testValue2);
  
  unsigned long tBefore = prop->GetMTime();
  prop->SetValue(testValue1);
  unsigned long tAfter = prop->GetMTime();
  prop->SetValue(testValue1);
  unsigned long tAfterAll = prop->GetMTime();
  
  if (prop->GetValue() != testValue1 || prop->GetValueAsString() != testValue1AsString) 
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
    
  std::cout << "Testing equality operator (operator==): ";
  if ( (! (*prop == *prop2)) || (*prop2 == *prop3) ) {
    std::cout << " [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  prop->SetValue(testValue2);
  unsigned long tAfterEverything = prop->GetMTime();
 
  std::cout << "Testing MTime correctness when changing property value: ";
  if (tBefore >= tAfter || tAfterAll != tAfter || tAfterEverything <= tAfterAll) {
     std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  prop->SetValue(testValue1);
  std::cout << "Testing assignment operator (operator=): ";
  *prop = *prop3;
  if ( (! (*prop == *prop3)) || (*prop == *prop2) ) {
    std::cout << " [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  return EXIT_SUCCESS;
}  


int mitkGenericPropertyTest(int /*argc*/, char* /*argv*/[])
{
  int retVal(EXIT_SUCCESS);

  TestGenericPropertyForDataType<int>(1, 2, "1", "2");
  TestGenericPropertyForDataType<bool>(true, false, "1", "0");

  std::cout << "[TEST DONE]"<<std::endl;
  return retVal;
}
