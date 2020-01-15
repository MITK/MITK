/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIPAGELAYOUTTEST_H_
#define BERRYIPAGELAYOUTTEST_H_

#include <berryUITestCase.h>

#include <cppunit/TestCaller.h>

namespace berry {

/**
 * Test cases for the <code>IPageLayout</code> API.
 *
 * @since 3.2
 */
class IPageLayoutTest : public UITestCase {

public:

  friend class CppUnit::TestCaller<IPageLayoutTest>;

  IPageLayoutTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestGetDescriptor();

};

}

#endif /* BERRYIPAGELAYOUTTEST_H_ */
