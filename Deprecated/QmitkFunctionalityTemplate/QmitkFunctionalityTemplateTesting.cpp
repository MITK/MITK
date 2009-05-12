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

#include "@FUNCTIONALITY_NAME@.h"

#ifdef BUILD_TESTING // only if we build a test driver

#include "@FUNCTIONALITY_NAME@Controls.h"

#include "mitkTestingMacros.h"
#include "QmitkUserInputSimulation.h"


/**
* \brief Test method for @FUNCTIONALITY_NAME@
*/
int @FUNCTIONALITY_NAME@::TestYourself()
{
  MITK_TEST_BEGIN("@FUNCTIONALITY_NAME@::TestYourself");

  // Write test here.
  // click on Buttons with QmitkUserInputSimulation::MouseClick(m_Controls->m_MyButton, Qt::LeftButton);
  // simulate key presses with QmitkUserInputSimulation::KeyboardTypeKey(m_Controls->m_MyWidget, Qt::Key_Down);
  // check results using MITK_TEST_CONDITION_REQUIRED(myBoolExpression ,"my testcase " << someVariable_that_should_be_mentioned);
  // and MITK_TEST_CONDITION(myBoolExpression ,"my testcase " << someVariable_that_should_be_mentioned);
  // Write Output to the command line only with MITK_TEST_OUTPUT(<< "Hello");
  // and MITK_TEST_OUTPUT_NO_ENDL(<< "Hello");
  // artificially fail a test with MITK_TEST_FAILED_MSG(<<"failed because!");
  // more information about writing a test can be found here: http://makalu/mbiwiki/TestsErstellen

  #ifndef MITK_FAST_TESTING
  // execute time consuming tests only in here (execution of time consuming filter pipeline...)

  #endif
 
  MITK_TEST_END();
}

#endif // BUILD_TESTING
