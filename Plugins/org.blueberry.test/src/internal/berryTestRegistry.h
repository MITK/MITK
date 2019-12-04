/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYTESTREGISTRY_H_
#define BERRYTESTREGISTRY_H_

#include <berryIConfigurationElement.h>

#include "berryITestDescriptor.h"

namespace berry {

class TestRegistry
{
public:

  static const QString TAG_TEST; // = "test"
  static const QString ATT_ID; // = "id"
  static const QString ATT_CLASS; // = "class"
  static const QString ATT_DESCRIPTION; // = "description"
  static const QString ATT_UITEST; // = "uitest"

  TestRegistry();

  const QList<ITestDescriptor::Pointer>& GetTestsForId(const QString& pluginid);

protected:

  void ReadTest(const IConfigurationElement::Pointer& testElem);

private:

  QHash<QString, QList<ITestDescriptor::Pointer> > mapIdToTests;
};

}

#endif /* BERRYTESTREGISTRY_H_ */
