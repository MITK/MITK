/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYSMARTPOINTERTEST_H_
#define CHERRYSMARTPOINTERTEST_H_

#include <CppUnit/TestCase.h>

namespace cherry {

class SmartPointerTest : public CppUnit::TestCase
{
public:

  SmartPointerTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestConstructors();
  void TestCasting();
  void TestReferenceCounting();
  void TestOperators();

};

}

#endif /* CHERRYSMARTPOINTERTEST_H_ */
