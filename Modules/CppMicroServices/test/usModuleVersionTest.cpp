/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleVersion.h>

#include "usTestingMacros.h"

#include <stdexcept>

using namespace us;

int usModuleVersionTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleVersionTest");

  ModuleVersion defined(1, 2, 3);
  ModuleVersion sameDefined(1, 2, 3);
  ModuleVersion otherDefined(2, 0, 0);
  ModuleVersion undefined = ModuleVersion::UndefinedVersion();
  ModuleVersion otherUndefined = ModuleVersion::UndefinedVersion();

  // Reflexive equality.
  US_TEST_CONDITION(defined == defined, "defined == itself")
  US_TEST_CONDITION(undefined == undefined, "undefined == itself")

  // Symmetric equality for two defined versions.
  US_TEST_CONDITION(defined == sameDefined, "defined == equal defined")
  US_TEST_CONDITION(sameDefined == defined, "defined == equal defined (reversed)")
  US_TEST_CONDITION(!(defined == otherDefined), "defined != different defined")
  US_TEST_CONDITION(!(otherDefined == defined), "defined != different defined (reversed)")

  // Two undefined versions compare equal.
  US_TEST_CONDITION(undefined == otherUndefined, "undefined == undefined")

  // Regression: comparing undefined with defined must be symmetric and must
  // NOT throw. Previously (undefined == defined) threw std::logic_error while
  // (defined == undefined) silently returned false.
  bool definedEqUndefined = false;
  bool undefinedEqDefined = false;
  try
  {
    definedEqUndefined = (defined == undefined);
    undefinedEqDefined = (undefined == defined);
  }
  catch (const std::logic_error&)
  {
    US_TEST_FAILED_MSG(<< "equality of defined and undefined versions must not throw")
  }
  US_TEST_CONDITION(!definedEqUndefined, "defined == undefined is false")
  US_TEST_CONDITION(!undefinedEqDefined, "undefined == defined is false")
  US_TEST_CONDITION(definedEqUndefined == undefinedEqDefined, "equality is symmetric across defined/undefined")

  US_TEST_END()
}
