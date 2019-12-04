/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryUiTestSuite.h"

#include "api/berryUiApiTestSuite.h"

#include <stdexcept>

namespace berry {

UiTestSuite::UiTestSuite()
: CppUnit::TestSuite("UiTestSuite")
{
  addTest(new UiApiTestSuite());
}

UiTestSuite::UiTestSuite(const UiTestSuite& other)
{
  Q_UNUSED(other);
  throw std::runtime_error("Copy constructor not implemented");
}

}
