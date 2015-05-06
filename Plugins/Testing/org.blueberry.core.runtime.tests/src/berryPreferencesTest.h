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


#ifndef BERRYPREFERENCESTEST_H_
#define BERRYPREFERENCESTEST_H_

#include <berryTestCase.h>

namespace berry {

class PreferencesTest : public berry::TestCase
{
public:

  PreferencesTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestAll();

};

}

#endif /* BERRYPREFERENCESTEST_H_ */
