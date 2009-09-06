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

#include "cherryMessageTest.h"

#include <cherryMessage.h>
//#include <cstdlib>

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

namespace cherry
{

struct MessageReceiver
{
  bool received;

  MessageReceiver() : received(false) {}

  void PureSignal() { received = true; }
  bool PureSignalWithReturn() { received = true; return received; }

  void Message1(int i) { received = (i != 0); }
  bool Message1WithReturn(int i) { received = (i != 0); return received; }

  void Message2(int i, float d) { received = (i != 0 && d != 0); }
  bool Message2WithReturn(int i, float d) { received = (i != 0 && d!= 0); return received; }

  void Message3(int i, float d, double e) { received = (i != 0 && d != 0 && e != 0); }
  bool Message3WithReturn(int i, float d, double e) { received = (i != 0 && d != 0 && e != 0); return received; }

  void Message4(int i, float d, double e, bool b) { received = (i != 0 && d != 0 && e != 0 && b); }
  bool Message4WithReturn(int i, float d, double e, bool b) { received = (i != 0 && d != 0 && e != 0 && b); return received; }

};

MessageTest::MessageTest(const std::string& testName)
: CppUnit::TestCase(testName)
{

}

CppUnit::Test* MessageTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("MessageTest");

  CppUnit_addTest(suite, MessageTest, TestMessage);
  CppUnit_addTest(suite, MessageTest, TestMessageWithReturn);
  CppUnit_addTest(suite, MessageTest, TestMessage1);
  CppUnit_addTest(suite, MessageTest, TestMessage1WithReturn);
  CppUnit_addTest(suite, MessageTest, TestMessage2);
  CppUnit_addTest(suite, MessageTest, TestMessage2WithReturn);
  CppUnit_addTest(suite, MessageTest, TestMessage3);
  CppUnit_addTest(suite, MessageTest, TestMessage3WithReturn);
  CppUnit_addTest(suite, MessageTest, TestMessage4);
  CppUnit_addTest(suite, MessageTest, TestMessage4WithReturn);

  return suite;
}

  void MessageTest::TestMessage()
  {
    cherry::Message<> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate<MessageReceiver> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::PureSignal);
    assertEqual(1, msg.GetListeners().size());
    msg += Delegate(&receiver2, &MessageReceiver::PureSignal);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::PureSignal);
    assertEqual(2, msg.GetListeners().size());

    msg.Send();
    assert(receiver.received && receiver2.received);

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::PureSignal);
    assertEqual(1, msg.GetListeners().size());

    msg.Send();
    assert(receiver.received == false && receiver2.received);
  }

  void MessageTest::TestMessageWithReturn()
  {
    cherry::Message<bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate<MessageReceiver, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::PureSignalWithReturn);
    msg();
    assert(receiver.received);

    receiver.received = false;
    typedef cherry::Message<bool>::ListenerList Listeners;
    const Listeners& listeners = msg.GetListeners();
    bool valueReturned = false;
    for (Listeners::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      valueReturned = (*iter)->Execute();
      if (valueReturned) break;
    }
    assert(valueReturned);
  }

  void MessageTest::TestMessage1()
  {
    cherry::Message1<int> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate1<MessageReceiver, int> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message1);
    assertEqual(1, msg.GetListeners().size());
    msg += Delegate(&receiver2, &MessageReceiver::Message1);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message1);
    assertEqual(2, msg.GetListeners().size());

    msg.Send(1);
    assert(receiver.received && receiver2.received);

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message1);
    assertEqual(1, msg.GetListeners().size());

    msg.Send(1);
    assert(receiver.received == false && receiver2.received);
  }

  void MessageTest::TestMessage1WithReturn()
  {
    cherry::Message1<int, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate1<MessageReceiver, int, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message1WithReturn);
    msg(1);
    assert(receiver.received);

    receiver.received = false;
    typedef cherry::Message1<int,bool>::ListenerList Listeners;
    const Listeners& listeners = msg.GetListeners();
    bool valueReturned = false;
    for (Listeners::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      valueReturned = (*iter)->Execute(1);
      if (valueReturned) break;
    }
    assert(valueReturned);
  }

  void MessageTest::TestMessage2()
  {
    cherry::Message2<int, float> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate2<MessageReceiver, int, float> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message2);
    assertEqual(1, msg.GetListeners().size());
    msg += Delegate(&receiver2, &MessageReceiver::Message2);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message2);
    assertEqual(2, msg.GetListeners().size());

    msg.Send(1, 1.0);
    assert(receiver.received && receiver2.received);

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message2);
    assertEqual(1, msg.GetListeners().size());

    msg.Send(1, 1.0);
    assert(receiver.received == false && receiver2.received);
  }

  void MessageTest::TestMessage2WithReturn()
  {
    cherry::Message2<int, float, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate2<MessageReceiver, int, float, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message2WithReturn);
    msg(1, 2);
    assert(receiver.received);

    receiver.received = false;
    typedef cherry::Message2<int,float,bool>::ListenerList Listeners;
    const Listeners& listeners = msg.GetListeners();
    bool valueReturned = false;
    for (Listeners::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      valueReturned = (*iter)->Execute(1, 0.4);
      if (valueReturned) break;
    }
    assert(valueReturned);
  }

  void MessageTest::TestMessage3()
  {
    cherry::Message3<int, float, double> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate3<MessageReceiver, int, float, double> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message3);
    assertEqual(1, msg.GetListeners().size());
    msg += Delegate(&receiver2, &MessageReceiver::Message3);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message3);
    assertEqual(2, msg.GetListeners().size());

    msg.Send(1, 1.0, 2.0);
    assert(receiver.received && receiver2.received);

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message3);
    assertEqual(1, msg.GetListeners().size());

    msg.Send(1, -1.0, 5.0);
    assert(receiver.received == false && receiver2.received);
  }

  void MessageTest::TestMessage3WithReturn()
  {
    cherry::Message3<int, float, double, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate3<MessageReceiver, int, float, double, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message3WithReturn);
    msg(1, -2, 0.2);
    assert(receiver.received);

    receiver.received = false;
    typedef cherry::Message3<int,float,double,bool>::ListenerList Listeners;
    const Listeners& listeners = msg.GetListeners();
    bool valueReturned = false;
    for (Listeners::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      valueReturned = (*iter)->Execute(1, 23.5, -12.2);
      if (valueReturned) break;
    }
    assert(valueReturned);
  }

  void MessageTest::TestMessage4()
  {
    cherry::Message4<int, float, double, bool> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate4<MessageReceiver, int, float, double, bool> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message4);
    assertEqual(1, msg.GetListeners().size());
    msg += Delegate(&receiver2, &MessageReceiver::Message4);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message4);
    assertEqual(2, msg.GetListeners().size());

    msg.Send(1, 5.4, -1.0, true);
    assert(receiver.received && receiver2.received);

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message4);
    assertEqual(1, msg.GetListeners().size());

    msg.Send(1, 0.2, 12.0, true);
    assert(receiver.received == false && receiver2.received);
  }

  void MessageTest::TestMessage4WithReturn()
  {
    cherry::Message4<int, float, double, bool, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate4<MessageReceiver, int, float, double, bool, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message4WithReturn);
    msg(1, 4.1, -1, true);
    assert(receiver.received);

    receiver.received = false;
    typedef cherry::Message4<int,float,double,bool,bool>::ListenerList Listeners;
    const Listeners& listeners = msg.GetListeners();
    bool valueReturned = false;
    for (Listeners::const_iterator iter = listeners.begin();
         iter != listeners.end(); ++iter)
    {
      valueReturned = (*iter)->Execute(1, -34.21, 2, true);
      if (valueReturned) break;
    }
    assert(valueReturned);
  }


}
