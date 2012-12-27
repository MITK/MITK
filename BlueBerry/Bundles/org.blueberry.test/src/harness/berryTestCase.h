/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYTESTCASE_H_
#define BERRYTESTCASE_H_

#include <CppUnit/TestCase.h>

#include <org_blueberry_test_Export.h>

namespace berry {

class BERRY_TEST_EXPORT TestCase : public CppUnit::TestCase
{

public:

  TestCase(const std::string& testName);

  /**
   * Sets up the fixture, for example, open a network connection.
   * This method is called before a test is executed.
   * The default implementation does nothing.
   * Subclasses may extend.
   */
  virtual void DoSetUp();

  /**
   * Tears down the fixture, for example, close a network connection.
   * This method is called after a test is executed.
   * The default implementation closes all test windows, processing events both before
   * and after doing so.
   * Subclasses may extend.
   */
  virtual void DoTearDown();

  /**
   * Clients should overwrite DoSetUp() instead of this method.
   */
  void setUp();

  /**
   * Clients should overwrite DoSetUp() instead of this method.
   */
  void tearDown();

protected:

  /**
   * Call this method in your unit test to enable detailed
   * output about leaking berry::Object instances.
   */
  void LeakDetailsOn();

  /**
   * Call this method to ignore leaking objects and to continue
   * with the unit tests.
   */
  void IgnoreLeakingObjects();

private:

  bool m_LeakDetails;
  bool m_IgnoreLeakage;

};

}

#endif /* BERRYTESTCASE_H_ */
