/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKPARAMETERIZEDTESTCALLER_H
#define MITKPARAMETERIZEDTESTCALLER_H

#include "cppunit/TestCase.h"

#include <vector>
#include <string>

extern std::vector<std::string> globalCmdLineArgs;

namespace mitk {

template<class ParameterizedFixture>
class ParameterizedTestCaller : public CppUnit::TestCase
{
  typedef void (ParameterizedFixture::*TestMethod)();

public:

  /**
   * Constructor for TestCaller. This constructor builds a new ParameterizedFixture
   * instance owned by the ParameterizedTestCaller.
   * \param name name of this ParameterizedTestCaller
   * \param test the method this ParameterizedTestCaller calls in runTest()
   */
  ParameterizedTestCaller(const std::string& name, TestMethod test)
    : TestCase(name)
    , m_OwnFixture(true)
    , m_Fixture(new ParameterizedFixture())
    , m_Test(test)
    , m_Parameter(globalCmdLineArgs)
  {
  }

  /**
   * Constructor for ParameterizedTestCaller.
   * This constructor does not create a new ParameterizedFixture instance but accepts
   * an existing one as parameter. The ParameterizedTestCaller will not own the
   * ParameterizedFixture object.
   * \param name name of this TestCaller
   * \param test the method this TestCaller calls in runTest()
   * \param fixture the Fixture to invoke the test method on.
   */
  ParameterizedTestCaller(const std::string& name, TestMethod test, ParameterizedFixture& fixture)
    : TestCase(name)
    , m_OwnFixture(false)
    , m_Fixture(&fixture)
    , m_Test(test)
    , m_Parameter(globalCmdLineArgs)
  {
  }

  /**
   * Constructor for ParameterizedTestCaller.
   * This constructor does not create a new ParameterizedFixture instance but accepts
   * an existing one as parameter. The ParameterizedTestCaller will own the
   * ParameterizedFixture object and delete it in its destructor.
   * \param name name of this TestCaller
   * \param test the method this TestCaller calls in runTest()
   * \param fixture the Fixture to invoke the test method on.
   */
  ParameterizedTestCaller(const std::string& name, TestMethod test, ParameterizedFixture* fixture)
    : TestCase(name)
    , m_OwnFixture(true)
    , m_Fixture(fixture)
    , m_Test(test)
    , m_Parameter(globalCmdLineArgs)
    {
    }

  /**
   * Constructor for ParameterizedTestCaller.
   * This constructor does not create a new ParameterizedFixture instance but accepts
   * an existing one as parameter. The ParameterizedTestCaller will own the
   * ParameterizedFixture object and delete it in its destructor.
   * \param name name of this TestCaller
   * \param test the method this TestCaller calls in runTest()
   * \param fixture the Fixture to invoke the test method on.
   * \param param A list of string parameters for the fixture.
   */
  ParameterizedTestCaller(const std::string& name, TestMethod test, ParameterizedFixture* fixture,
                          const std::vector<std::string>& param)
    : TestCase(name)
    , m_OwnFixture(true)
    , m_Fixture(fixture)
    , m_Test(test)
    , m_Parameter(param)
    {
    }

  ~ParameterizedTestCaller()
  {
    if (m_OwnFixture)
      delete m_Fixture;
  }

  void runTest()
  {
    (m_Fixture->*m_Test)();
  }

  void setUp()
  {
    if (!m_Parameter.empty())
    {
      m_Fixture->setUpParameter(m_Parameter);
    }
    m_Fixture->setUp();
  }

  void tearDown()
  {
    m_Fixture->tearDown();
  }

  std::string toString() const
  {
    return "TestCaller " + getName();
  }

private:
  ParameterizedTestCaller(const ParameterizedTestCaller& other);
  ParameterizedTestCaller& operator =(const ParameterizedTestCaller& other);

private:
  bool m_OwnFixture;
  ParameterizedFixture* m_Fixture;
  TestMethod m_Test;
  std::vector<std::string> m_Parameter;
};

}

#endif // MITKPARAMETERIZEDTESTCALLER_H
