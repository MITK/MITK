/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkLookupTables.h"
#include "mitkStringProperty.h"
#include <iostream>

/// \TODO DataTree based scene writing is deprecated and will be replaced by a new mechanism. Therefore
/// This test case is disabled for now.
//bool TestXMLWriter()
//{
//#define FIRST_VALUE  5
//#define SECOND_VALUE 325
//
//  std::cout << "Testing to share properties between two PropertyLists..." << std::endl;
//
//  // first list
//  mitk::DataTreeNode::Pointer list1 = mitk::DataTreeNode::New();
//  // insert an IntProperty
//  mitk::IntProperty::Pointer intProp = mitk::IntProperty::New(FIRST_VALUE);
//  list1->SetProperty("int", intProp);
//  
//  // second list
//  mitk::DataTreeNode::Pointer list2 = mitk::DataTreeNode::New();
//  // insert the same property as in list1
//  list2->SetProperty("int", intProp);
//
//  // everything ok?
//  mitk::IntProperty* ip1 = dynamic_cast<mitk::IntProperty*>( list1->GetProperty("int") );
//
//  if (!ip1)
//  {
//    std::cout << "  IntProperty got lost after SetProperty()" << std::endl;
//    return false;
//  }
//
//  if (ip1->GetValue() != FIRST_VALUE)
//  {
//    std::cout << "  IntProperty got changed SetProperty()" << std::endl;
//    return false;
//   }
//  
//  mitk::IntProperty* ip2 = dynamic_cast<mitk::IntProperty*>( list2->GetProperty("int"));
//
//  if (ip1 != ip2)
//  {
//    std::cout << "  pointers to IntProperties somehow corrupted" << std::endl;
//    return false;
//  }
//
//  if (ip1->GetValue() != ip2->GetValue())
//  {
//    std::cout << "  IntProperties somehow corrupted" << std::endl;
//    return false;
//  }
//
//  ip1->SetValue(SECOND_VALUE);
//  if (ip1->GetValue() != ip2->GetValue())
//  {
//    std::cout << "  IntProperties somehow corrupted" << std::endl;
//    return false;
//  }
// 
//  std::cout << "Writing lists to file" << std::endl;
//  // write to file
//  {
//    // build a small data tree to write to file
//    mitk::DataTree::Pointer tree = mitk::DataTree::New();
//    mitk::DataTreePreOrderIterator iter(tree);
//    iter.Add(list1);
//    iter.Add(list2);
//
//    mitk::DataTree::Save(&iter, "tempfile", false);
//  }
//  
//  std::cout << "Load lists from file again" << std::endl;
//  // read from file
//  mitk::DataTree::Pointer tree = mitk::DataTree::New();
//  mitk::DataTreePreOrderIterator iter(tree);
//  mitk::DataTree::Load(&iter, "tempfile");
//
//  iter.GoToBegin();
//
//  ++iter;
//  list1 = iter.Get();
//  ++iter;
//  list2 = iter.Get();
//
//  std::cout << "Test if everything is still as before writing..." << std::endl;
//   // everything ok?
//
//  ip1 = dynamic_cast<mitk::IntProperty*>( list1->GetProperty("int"));
//  ip2 = dynamic_cast<mitk::IntProperty*>( list2->GetProperty("int"));
//
//  if (!ip1)
//  {
//    std::cout << "  IntProperty got lost after write/read to file" << std::endl;
//    return false;
//  }
//
//  if (ip1->GetValue() != SECOND_VALUE)
//  {
//    std::cout << "  IntProperty got changed write/read to file" << std::endl;
//    return false;
//   }
//  
//  ip2 = dynamic_cast<mitk::IntProperty*>( list2->GetProperty("int"));
//
//  if (ip1 != ip2)
//  {
//    std::cout << "  pointers to IntProperties somehow corrupted" << std::endl;
//    return false;
//  }
//
//  if (ip1->GetValue() != ip2->GetValue())
//  {
//    std::cout << "  IntProperties somehow corrupted" << std::endl;
//    return false;
//  }
//
//  ip1->SetValue(FIRST_VALUE);
//  if (ip1->GetValue() != ip2->GetValue())
//  {
//    std::cout << "  IntProperties somehow corrupted" << std::endl;
//    return false;
//  }
//  
//  return true;
//}

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
  mitk::BoolProperty::Pointer boolProp = mitk::BoolProperty::New(false);
  mitk::BoolProperty::Pointer boolProp2 = mitk::BoolProperty::New(false);
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
  propList->SetProperty("test",mitk::BoolProperty::New(true)); 
  tAfter = propList->GetMTime();
  if (!  (tAfter > tBefore) ) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  std::cout << "Testing SetProperty() with unchanged property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",mitk::BoolProperty::New(true)); 
  tAfter = propList->GetMTime();
  if ( tBefore != tAfter ) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  
  std::cout << "Testing MTime correctness when changing property value: ";
  boolProp = mitk::BoolProperty::New(true);
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
  boolProp = mitk::BoolProperty::New(true);
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
  
  //std::cout << "Testing output of PropertyList to file: ";
  //if ( TestXMLWriter() )
  //  std::cout << "[PASSED]" << std::endl;
  //else
  //  return EXIT_FAILURE;
  
  std::cout << "Testing GetPropertyValue(bool): ";
  mitk::BoolProperty::Pointer gpvTest = mitk::BoolProperty::New(true);
  propList->SetProperty("gpvBool", gpvTest);
  bool b = false;
  if ((propList->GetPropertyValue<bool>("gpvBool", b) == true) && (b == gpvTest->GetValue()))
    std::cout << "[PASSED]" << std::endl;
  else
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  
  std::cout << "Testing GetPropertyValue(float): ";
  mitk::FloatProperty::Pointer gpvTest2 = mitk::FloatProperty::New(3.1337);
  propList->SetProperty("gpvfloat", gpvTest2);
  float v = -1.23;
  if ((propList->GetPropertyValue<float>("gpvfloat", v) == true) && (v == gpvTest2->GetValue()))
    std::cout << "[PASSED]" << std::endl;
  else
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing GetPropertyValue(BoolLookupTable): ";
  mitk::BoolLookupTable blt;
  blt.SetTableValue(17, true);
  propList->SetProperty("blutprop", mitk::BoolLookupTableProperty::New(blt));
  try
  {
    mitk::BoolLookupTable blut;
    if ((propList->GetPropertyValue<mitk::BoolLookupTable>("blutprop", blut) == true) && (blut.GetTableValue(17) == true))
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout << "Exception thrown! [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  {  
    std::cout << "Testing GetBoolProperty(): ";
    mitk::BoolProperty::Pointer prop = mitk::BoolProperty::New(true);
    propList->ReplaceProperty("test", prop);
    bool v = false;
    if ((propList->GetBoolProperty("test", v) == true) && (v == prop->GetValue()))
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
  }
  {  
    std::cout << "Testing GetIntProperty(): ";
    mitk::IntProperty::Pointer prop = mitk::IntProperty::New(31337);
    propList->ReplaceProperty("test", prop);
    int v = 1;
    if ((propList->GetIntProperty("test", v) == true) && (v == prop->GetValue()))
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
  }
  {  
    std::cout << "Testing GetFloatProperty(): ";
    mitk::FloatProperty::Pointer prop = mitk::FloatProperty::New(31.337);
    propList->ReplaceProperty("test", prop);
    float v = 1.2;
    if ((propList->GetFloatProperty("test", v) == true) && (v == prop->GetValue()))
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
  } 
  {  
    std::cout << "Testing GetStringProperty(): ";
    mitk::StringProperty::Pointer prop = mitk::StringProperty::New("MITK");
    propList->ReplaceProperty("test", prop);
    std::string v = "";
    if ((propList->GetStringProperty("test", v) == true) && (v == prop->GetValue()))
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
  } 
  
  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
