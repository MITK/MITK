/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYMESSAGETEST_H_
#define BERRYMESSAGETEST_H_

#include <berryTestCase.h>

namespace berry {

class MessageTest : public TestCase {

public:

  MessageTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestMessage();
  void TestMessageWithReturn();
  void TestMessage1();
  void TestMessage1WithReturn();
  void TestMessage2();
  void TestMessage2WithReturn();
  void TestMessage3();
  void TestMessage3WithReturn();
  void TestMessage4();
  void TestMessage4WithReturn();

};

}

#endif /* BERRYMESSAGETEST_H_ */
