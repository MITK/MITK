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


#ifndef BERRYIWORKBENCHPARTTEST_H_
#define BERRYIWORKBENCHPARTTEST_H_

#include <berryUITestCase.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

#include "berryMockWorkbenchPart.h"

namespace berry {

/**
 * This is a test for IWorkbenchPart.  Since IWorkbenchPart is an
 * interface this test verifies the IWorkbenchPart lifecycle rather
 * than the implementation.
 */
class IWorkbenchPartTest : public UITestCase {

protected:

  IWorkbenchWindow::Pointer fWindow;
  IWorkbenchPage::Pointer fPage;

  void DoSetUp();

  /**
     * Opens a part.  Subclasses should override
     */
   virtual MockWorkbenchPart::Pointer OpenPart(IWorkbenchPage::Pointer page) = 0;

    /**
     * Closes a part.  Subclasses should override
     */
    virtual void ClosePart(IWorkbenchPage::Pointer page, MockWorkbenchPart::Pointer part) = 0;

public:

    /**
     * Constructor for IActionDelegateTest
     */
    IWorkbenchPartTest(const std::string& testName);

    void TestOpenAndClose();

};

}

#endif /* BERRYIWORKBENCHPARTTEST_H_ */
