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

#ifndef BERRYIVIEWPARTTEST_H_
#define BERRYIVIEWPARTTEST_H_

#include "berryIWorkbenchPartTest.h"

#include <CppUnit/TestCaller.h>

namespace berry
{

/**
 * This is a test for IViewPart.  Since IViewPart is an
 * interface this test verifies the IViewPart lifecycle rather
 * than the implementation.
 */
class IViewPartTest: public IWorkbenchPartTest
{

public:

  friend class CppUnit::TestCaller<IViewPartTest>;

  /**
   * Constructor for IViewPartTest
   */
  IViewPartTest(const std::string& testName);

  static CppUnit::Test* Suite();

  /**
   * Tests that the view is closed without saving if isSaveOnCloseNeeded()
   * returns false. This also tests some disposal behaviors specific to
   * views: namely, that the contribution items are disposed in the correct
   * order with respect to the disposal of the view.
   *
   * @see ISaveablePart#isSaveOnCloseNeeded()
   */
  void TestOpenAndCloseSaveNotNeeded();

protected:

  /**
   * @see IWorkbenchPartTest#openPart(IWorkbenchPage)
   */
  MockWorkbenchPart::Pointer OpenPart(IWorkbenchPage::Pointer page);

  /**
   * @see IWorkbenchPartTest#closePart(IWorkbenchPage, MockWorkbenchPart)
   */
  void ClosePart(IWorkbenchPage::Pointer page, MockWorkbenchPart::Pointer part);

};

}

#endif /* BERRYIVIEWPARTTEST_H_ */
