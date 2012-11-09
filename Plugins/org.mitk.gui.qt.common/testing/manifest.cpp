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

#include <Poco/ClassLibrary.h>

#include "src/QmitkQtCommonTestSuite.h"

#include "src/api/QmitkMockFunctionality.h"

POCO_BEGIN_NAMED_MANIFEST(CppUnitTest, CppUnit::Test)
  POCO_EXPORT_CLASS(QmitkQtCommonTestSuite)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(berryIViewPart, berry::IViewPart)
  POCO_EXPORT_CLASS(QmitkMockFunctionality)
POCO_END_MANIFEST

