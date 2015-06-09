/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
