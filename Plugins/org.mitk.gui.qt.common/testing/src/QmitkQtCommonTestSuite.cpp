/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
