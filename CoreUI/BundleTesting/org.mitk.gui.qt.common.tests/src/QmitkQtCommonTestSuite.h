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
