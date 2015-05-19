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
