/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

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
