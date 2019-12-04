/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryUiApiTestSuite.h"

#include "berryXMLMementoTest.h"
#include "berryIPageLayoutTest.h"
#include "berryIViewPartTest.h"

namespace berry {

UiApiTestSuite::UiApiTestSuite()
: CppUnit::TestSuite("UiApiTestSuite")
{
  addTest(IPageLayoutTest::Suite());
  addTest(IViewPartTest::Suite());
  addTest(XMLMementoTest::Suite());
}

}
