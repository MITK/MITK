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
