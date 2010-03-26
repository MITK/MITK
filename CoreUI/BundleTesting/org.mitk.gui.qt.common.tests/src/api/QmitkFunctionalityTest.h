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
