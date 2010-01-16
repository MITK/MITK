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
