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


#ifndef BERRYCORERUNTIMETESTSUITE_H_
#define BERRYCORERUNTIMETESTSUITE_H_

#include <CppUnit/TestSuite.h>

#include <QObject>

Q_DECLARE_INTERFACE(CppUnit::Test, "CppUnit.Test")

namespace berry {

class CoreRuntimeTestSuite : public QObject, public CppUnit::TestSuite
{
  Q_OBJECT
  Q_INTERFACES(CppUnit::Test)

public:

  CoreRuntimeTestSuite();
  CoreRuntimeTestSuite(const CoreRuntimeTestSuite& other);
};

}

#endif /* BERRYCORERUNTIMETESTSUITE_H_ */
