/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkLookupTables.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkStringProperty.h"
#include "mitkExceptionMacro.h"

#include <iostream>

int mitkPropertyListTest(int /*argc*/, char * /*argv*/ [])
{
  mitk::PropertyList::Pointer propList;
  std::cout << "Testing mitk::PropertyList::New(): ";
  propList = mitk::PropertyList::New();
  if (propList.IsNull())
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }
  mitk::BoolProperty::Pointer boolProp = mitk::BoolProperty::New(false);
  mitk::BoolProperty::Pointer boolProp2 = mitk::BoolProperty::New(false);
  std::cout << "Testing BoolProperty ==: ";
  if (!(*boolProp2 == *boolProp))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;
  unsigned long tBefore, tAfter;

  std::cout << "Testing SetProperty() with new key value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test", boolProp);
  tAfter = propList->GetMTime();
  if (!(tAfter > tBefore))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing SetProperty() with changed property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test", mitk::BoolProperty::New(true));
  tAfter = propList->GetMTime();
  if (!(tAfter > tBefore))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing SetProperty() with unchanged property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test", mitk::BoolProperty::New(true));
  tAfter = propList->GetMTime();
  if (tBefore != tAfter)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing MTime correctness when changing property value: ";
  boolProp = mitk::BoolProperty::New(true);
  propList->ReplaceProperty("test", boolProp);
  tBefore = propList->GetMTime();
  boolProp->SetValue(true);
  tAfter = propList->GetMTime();
  boolProp->SetValue(false);
  unsigned long tAfterAll = propList->GetMTime();

  if (tBefore != tAfter || tAfterAll <= tAfter)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing MTime correctness when calling SetProperty twice: ";
  boolProp = mitk::BoolProperty::New(true);
  propList->SetProperty("test", boolProp);
  tBefore = propList->GetMTime();
  propList->SetProperty("test", boolProp);
  tAfter = propList->GetMTime();

  if (tBefore != tAfter)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing if existing properties survive SetProperty: ";
  propList->SetProperty("test", boolProp);
  mitk::BaseProperty *bpBefore = propList->GetProperty("test");
  propList->SetProperty("test", boolProp2);
  mitk::BaseProperty *bpAfter = propList->GetProperty("test");

  if (bpBefore != bpAfter || bpAfter == nullptr)
  {
    std::cout << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing if existing properties survive ReplaceProperty: ";
  propList->SetProperty("test", boolProp);
  bpBefore = propList->GetProperty("test");
  propList->ReplaceProperty("test", boolProp2);
  bpAfter = propList->GetProperty("test");

  if (bpBefore == bpAfter || bpAfter == nullptr)
  {
    std::cout << std::endl;
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  // std::cout << "Testing output of PropertyList to file: ";
  // if ( TestXMLWriter() )
  //  std::cout << "[PASSED]" << std::endl;
  // else
  //  return EXIT_FAILURE;

  std::cout << "Testing GetPropertyValue(bool): ";
  mitk::BoolProperty::Pointer gpvTest = mitk::BoolProperty::New(true);
  propList->SetProperty("gpvBool", gpvTest);
  bool b = false;
  bool getPropertyValueReturnValue = propList->GetPropertyValue<bool>("gpvBool", b);
  if ((getPropertyValueReturnValue == true) && (b == gpvTest->GetValue()))
    std::cout << "[PASSED]" << std::endl;
  else
  {
    std::cout << "Oh, not goot:"
                 "\nWe called propList->GetPropertyValue<bool>('gpvBool', b) and it returned "
              << getPropertyValueReturnValue << "\nThen we compared b [" << b << "] and gpvTest->GetValue() ["
              << gpvTest->GetValue() << "]" << std::endl;
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
    if ((propList->GetPropertyValue<mitk::BoolLookupTable>("blutprop", blut) == true) &&
        (blut.GetTableValue(17) == true))
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
  }
  catch (...)
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
  {

    std::cout << "Testing RemoveProperty(): ";
    propList->RemoveProperty("test");
    if (propList->GetProperty("test")!=nullptr)
    {
      std::cout << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "[PASSED]" << std::endl;
  }

  std::cout << "Testing SetProperty() with no property (nullptr): ";
  tBefore = propList->GetMTime();
  propList->SetProperty("nullprop", nullptr);
  tAfter = propList->GetMTime();
  if (!(tAfter == tBefore))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "Testing SetProperty() with invalid (empty) name: ";
  try
  {
    propList->SetProperty("", boolProp);
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const mitk::Exception& /*e*/)
  {
    std::cout << "[PASSED]" << std::endl;
  }
  catch (...)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
