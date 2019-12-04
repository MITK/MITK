/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYBLUEBERRYTESTDRIVER_H_
#define BERRYBLUEBERRYTESTDRIVER_H_

#include <org_blueberry_test_Export.h>

#include "berryITestDescriptor.h"

namespace berry
{

/**
 * A TestDriver for CppUnit that supports running tests inside BlueBerry as well as
 * running standalone.
 * Example call: TODO
 */
class BERRY_TEST_EXPORT BlueBerryTestDriver
{
public:

  BlueBerryTestDriver(const QList<ITestDescriptor::Pointer>& descriptors, bool uitests = false, const QString& testName="", bool wait=false);

  int Run();

  static int Run(const QString& pluginId, bool uitests = false);

protected:

  QList<ITestDescriptor::Pointer> descriptors;
  bool uitests;
  QString testName;
  bool wait;
};

}

#endif /* BERRYBLUEBERRYTESTDRIVER_H_ */
