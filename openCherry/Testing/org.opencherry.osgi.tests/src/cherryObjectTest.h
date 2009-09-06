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


#ifndef CHERRYOBJECTTEST_H_
#define CHERRYOBJECTTEST_H_

#include <CppUnit/TestCase.h>

namespace cherry {

class ObjectTest : public CppUnit::TestCase
{
public:

  static CppUnit::Test* Suite();

  ObjectTest(const std::string& testName);

  void TestReferenceCount();
  void TestAddDestroyListener();
  void TestRemoveDestroyListener();
};

}

#endif /* CHERRYOBJECTTEST_H_ */
