/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKFUNCTIONALITYTEST_H_
#define QMITKFUNCTIONALITYTEST_H_

#include <berryUITestCase.h>

#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

#include <CppUnit/TestCaller.h>

/**
 * Test cases for the <code>QmitkFunctionality</code> API.
 */
class QmitkFunctionalityTest : public berry::UITestCase {

public:

  friend class CppUnit::TestCaller<QmitkFunctionalityTest>;

  QmitkFunctionalityTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestOpenAndClose();
  void TestOpenAndWindowClose();

protected:

  berry::IWorkbenchWindow::Pointer fWindow;
  berry::IWorkbenchPage::Pointer fPage;

  void DoSetUp();
  void DoTearDown();


};


#endif /* QMITKFUNCTIONALITYTEST_H_ */
