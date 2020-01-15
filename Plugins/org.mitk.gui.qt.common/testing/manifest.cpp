/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <Poco/ClassLibrary.h>

#include "src/QmitkQtCommonTestSuite.h"

#include "src/api/QmitkMockFunctionality.h"

POCO_BEGIN_NAMED_MANIFEST(CppUnitTest, CppUnit::Test)
  POCO_EXPORT_CLASS(QmitkQtCommonTestSuite)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(berryIViewPart, berry::IViewPart)
  POCO_EXPORT_CLASS(QmitkMockFunctionality)
POCO_END_MANIFEST

