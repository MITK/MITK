/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkQtCommonTestSuite.h"

#include "api/QmitkUiApiTestSuite.h"

#include <stdexcept>

QmitkQtCommonTestSuite::QmitkQtCommonTestSuite()
: CppUnit::TestSuite("QmitkQtCommonTestSuite")
{
  addTest(new QmitkUiApiTestSuite());
}

QmitkQtCommonTestSuite::QmitkQtCommonTestSuite(const QmitkQtCommonTestSuite& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}
