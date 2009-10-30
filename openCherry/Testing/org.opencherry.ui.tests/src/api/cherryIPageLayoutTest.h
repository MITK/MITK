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


#ifndef CHERRYIPAGELAYOUTTEST_H_
#define CHERRYIPAGELAYOUTTEST_H_

#include <cherryUITestCase.h>

#include <CppUnit/TestCaller.h>

namespace cherry {

/**
 * Test cases for the <code>IPageLayout</code> API.
 *
 * @since 3.2
 */
class IPageLayoutTest : public UITestCase {

public:

  friend class CppUnit::TestCaller<IPageLayoutTest>;

  IPageLayoutTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestGetDescriptor();

};

}

#endif /* CHERRYIPAGELAYOUTTEST_H_ */
