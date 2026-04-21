/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModule.h>
#include <usModuleEvent.h>
#include <usServiceEvent.h>
#include <usServiceReference.h>

#include "usTestingMacros.h"

#include <sstream>
#include <stdexcept>

using namespace us;

int usNullEventTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("NullEventTest");

  // Regression: default-constructed ModuleEvent previously crashed in
  // GetModule()/GetType(). GetModule() now returns nullptr, GetType() throws.
  {
    ModuleEvent evt;
    US_TEST_CONDITION(evt.IsNull(), "default ModuleEvent is null")
    US_TEST_CONDITION(evt.GetModule() == nullptr,
                      "default ModuleEvent::GetModule() returns nullptr")
    US_TEST_FOR_EXCEPTION(std::logic_error, evt.GetType())
  }

  // Regression: default-constructed ServiceEvent previously crashed in
  // GetServiceReference()/GetType(). GetServiceReference() now returns an
  // invalid reference, GetType() throws.
  {
    ServiceEvent evt;
    US_TEST_CONDITION(evt.IsNull(), "default ServiceEvent is null")
    ServiceReferenceU ref = evt.GetServiceReference();
    US_TEST_CONDITION(!ref,
                      "default ServiceEvent::GetServiceReference() returns invalid ref")
    US_TEST_FOR_EXCEPTION(std::logic_error, evt.GetType())
  }

  // Regression: operator<<(ostream, Module*) previously dereferenced a null
  // pointer. It now writes "null".
  {
    std::ostringstream os;
    os << static_cast<Module*>(nullptr);
    US_TEST_CONDITION(os.str() == "null",
                      "operator<< on null Module* writes \"null\"")
  }

  US_TEST_END()
}
