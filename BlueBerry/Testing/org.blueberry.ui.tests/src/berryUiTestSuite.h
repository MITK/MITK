/*=========================================================================
 
 Program:   BlueBerry Platform
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


#ifndef BERRYUITESTSUITE_H_
#define BERRYUITESTSUITE_H_

#include <CppUnit/TestSuite.h>

#include <QObject>

Q_DECLARE_INTERFACE(CppUnit::Test, "CppUnit.Test")

namespace berry {

class UiTestSuite : public QObject, public CppUnit::TestSuite
{
  Q_OBJECT
  Q_INTERFACES(CppUnit::Test)

public:

  UiTestSuite();
  UiTestSuite(const UiTestSuite& other);
};

}

#endif /* BERRYUITESTSUITE_H_ */
