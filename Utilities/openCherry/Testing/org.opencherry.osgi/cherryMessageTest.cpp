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

#include <cherryTestingMacros.h>

#include <cherryMessage.h>
#include <cstdlib>

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

int cherryMessageTest(int /*argc*/, char* /*argv*/[])
{
  CHERRY_TEST_BEGIN("Message")

  // Message test
  {
    cherry::Message<> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate<MessageReceiver> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::PureSignal);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Adding listener")
    msg += Delegate(&receiver2, &MessageReceiver::PureSignal);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::PureSignal);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 2, "Adding duplicate listener")

    msg.Send();
    CHERRY_TEST_CONDITION(receiver.received && receiver2.received, "Receiving signal")

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::PureSignal);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Removing listener")

    msg.Send();
    CHERRY_TEST_CONDITION(receiver.received == false && receiver2.received, "Receiving signal")
  }

  // Message with return type test
  {
    cherry::Message<bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate<MessageReceiver, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::PureSignalWithReturn);
    msg();
    CHERRY_TEST_CONDITION(receiver.received, "Receiving signal with return type")

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
    CHERRY_TEST_CONDITION(valueReturned, "Handling return values")
  }

  // Message1 test
  {
    cherry::Message1<int> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate1<MessageReceiver, int> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message1);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Adding listener")
    msg += Delegate(&receiver2, &MessageReceiver::Message1);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message1);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 2, "Adding duplicate listener")

    msg.Send(1);
    CHERRY_TEST_CONDITION(receiver.received && receiver2.received, "Receiving message1")

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message1);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Removing listener")

    msg.Send(1);
    CHERRY_TEST_CONDITION(receiver.received == false && receiver2.received, "Receiving message1")
  }

  // Message1 with return type test
  {
    cherry::Message1<int, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate1<MessageReceiver, int, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message1WithReturn);
    msg(1);
    CHERRY_TEST_CONDITION(receiver.received, "Receiving message1 with return type")

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
    CHERRY_TEST_CONDITION(valueReturned, "Handling return values")
  }

  // Message2 test
  {
    cherry::Message2<int, float> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate2<MessageReceiver, int, float> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message2);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Adding listener")
    msg += Delegate(&receiver2, &MessageReceiver::Message2);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message2);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 2, "Adding duplicate listener")

    msg.Send(1, 1.0);
    CHERRY_TEST_CONDITION(receiver.received && receiver2.received, "Receiving message2")

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message2);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Removing listener")

    msg.Send(1, 1.0);
    CHERRY_TEST_CONDITION(receiver.received == false && receiver2.received, "Receiving message2")
  }

  // Message2 with return type test
  {
    cherry::Message2<int, float, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate2<MessageReceiver, int, float, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message2WithReturn);
    msg(1, 2);
    CHERRY_TEST_CONDITION(receiver.received, "Receiving message2 with return type")

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
    CHERRY_TEST_CONDITION(valueReturned, "Handling return values")
  }

  // Message3 test
  {
    cherry::Message3<int, float, double> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate3<MessageReceiver, int, float, double> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message3);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Adding listener")
    msg += Delegate(&receiver2, &MessageReceiver::Message3);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message3);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 2, "Adding duplicate listener")

    msg.Send(1, 1.0, 2.0);
    CHERRY_TEST_CONDITION(receiver.received && receiver2.received, "Receiving message3")

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message3);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Removing listener")

    msg.Send(1, -1.0, 5.0);
    CHERRY_TEST_CONDITION(receiver.received == false && receiver2.received, "Receiving message3")
  }

  // Message3 with return type test
  {
    cherry::Message3<int, float, double, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate3<MessageReceiver, int, float, double, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message3WithReturn);
    msg(1, -2, 0.2);
    CHERRY_TEST_CONDITION(receiver.received, "Receiving message3 with return type")

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
    CHERRY_TEST_CONDITION(valueReturned, "Handling return values")
  }

  // Message4 test
  {
    cherry::Message4<int, float, double, bool> msg;

    MessageReceiver receiver;
    MessageReceiver receiver2;

    typedef cherry::MessageDelegate4<MessageReceiver, int, float, double, bool> Delegate;

    msg += Delegate(&receiver, &MessageReceiver::Message4);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Adding listener")
    msg += Delegate(&receiver2, &MessageReceiver::Message4);
    // duplicate entry
    msg += Delegate(&receiver, &MessageReceiver::Message4);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 2, "Adding duplicate listener")

    msg.Send(1, 5.4, -1.0, true);
    CHERRY_TEST_CONDITION(receiver.received && receiver2.received, "Receiving message4")

    receiver.received = false;
    receiver2.received = false;
    msg -= Delegate(&receiver, &MessageReceiver::Message4);
    CHERRY_TEST_CONDITION(msg.GetListeners().size() == 1, "Removing listener")

    msg.Send(1, 0.2, 12.0, true);
    CHERRY_TEST_CONDITION(receiver.received == false && receiver2.received, "Receiving message4")
  }

  // Message4 with return type test
  {
    cherry::Message4<int, float, double, bool, bool> msg;

    MessageReceiver receiver;
    typedef cherry::MessageDelegate4<MessageReceiver, int, float, double, bool, bool> Delegate;
    msg += Delegate(&receiver, &MessageReceiver::Message4WithReturn);
    msg(1, 4.1, -1, true);
    CHERRY_TEST_CONDITION(receiver.received, "Receiving message4 with return type")

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
    CHERRY_TEST_CONDITION(valueReturned, "Handling return values")
  }

  CHERRY_TEST_END()
}
