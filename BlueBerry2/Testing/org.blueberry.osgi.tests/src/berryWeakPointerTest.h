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


#ifndef BERRYWEAKPOINTERTEST_H_
#define BERRYWEAKPOINTERTEST_H_

#include <berryTestCase.h>

namespace berry {

class WeakPointerTest : public TestCase
{
public:

  WeakPointerTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestReferenceCounting();
  void TestConstructorAndLock();
  void TestOperators();
};

}

#endif /* BERRYWEAKPOINTERTEST_H_ */
