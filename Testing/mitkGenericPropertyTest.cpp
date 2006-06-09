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

#include <fstream>
int mitkGenericPropertyTest(int /*argc*/, char* /*argv*/[])
{
  std::cout << "Testing mitk::GenericProperty<int>:";
  mitk::GenericProperty<int>::Pointer prop = new mitk::GenericProperty<int>();
  mitk::GenericProperty<int>::Pointer prop2 = new mitk::GenericProperty<int>(5);
  mitk::GenericProperty<int>::Pointer prop3 = new mitk::GenericProperty<int>(7);
  unsigned long tBefore = prop->GetMTime();
  prop->SetValue(5);
  unsigned long tAfter = prop->GetMTime();
  prop->SetValue(5);
  unsigned long tAfterAll = prop->GetMTime();
  
  if (prop->GetValue() != 5 || prop->GetValueAsString() != "5") {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
    
  std::cout << "Testing equality operation: ";
  
  if ( (! (*prop == *prop2)) || (*prop2 == *prop3) ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  std::cout << "Testing MTime correctness when changing property value: ";
  
  if (tBefore >= tAfter || tAfterAll != tAfter) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
