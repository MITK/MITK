/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYOSGICORETESTSUITE_H_
#define BERRYOSGICORETESTSUITE_H_

#include <cppunit/TestSuite.h>

#include <QObject>

Q_DECLARE_INTERFACE(CppUnit::Test, "CppUnit.Test")

namespace berry {

class OSGiCoreTestSuite : public QObject, public CppUnit::TestSuite
{
  Q_OBJECT
  Q_INTERFACES(CppUnit::Test)

public:

  OSGiCoreTestSuite();
  OSGiCoreTestSuite(const OSGiCoreTestSuite& other);
};

}

#endif /* BERRYOSGICORETESTSUITE_H_ */
