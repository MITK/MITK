/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-09-07 12:05:58 +0200 (Mo, 07 Sep 2009) $
 Version:   $Revision: 18832 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef CHERRYPREFERENCESTEST_H_
#define CHERRYPREFERENCESTEST_H_

#include <CppUnit/TestCase.h>

namespace cherry {

class PreferencesTest : public CppUnit::TestCase
{
public:

  PreferencesTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestAll();

};

}

#endif /* CHERRYPREFERENCESTEST_H_ */
