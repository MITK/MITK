/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKQTCOMMONTESTSUITE_H_
#define QMITKQTCOMMONTESTSUITE_H_

#include <CppUnit/TestSuite.h>

#include <QObject>

Q_DECLARE_INTERFACE(CppUnit::Test, "CppUnit.Test")

class QmitkQtCommonTestSuite : public QObject, public CppUnit::TestSuite
{
  Q_OBJECT
  Q_INTERFACES(CppUnit::Test)

public:

  QmitkQtCommonTestSuite();
  QmitkQtCommonTestSuite(const QmitkQtCommonTestSuite& other);
};

#endif /* QMITKQTCOMMONTESTSUITE_H_ */
