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


#ifndef BERRYPREFERENCESSERVICETEST_H_
#define BERRYPREFERENCESSERVICETEST_H_

#include <berryTestCase.h>

namespace berry {

class PreferencesServiceTest : public berry::TestCase
{
public:

  static CppUnit::Test* Suite();

  PreferencesServiceTest(const std::string& testName);

  void TestAll();
};

}

#endif /* BERRYPREFERENCESSERVICETEST_H_ */
