/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkMessage.h"

#include "mitkTestingMacros.h"

#include <iostream>

namespace mitk {

class mitkMessageTestTestClass
{

public:

  // dummy class to send around
  class Package
  {
  public:

    Package(int content = 43)
      :m_Content(content)
    {
    }

    void Clear()
    {
      m_Content = 0;
    }

    bool operator==(const Package& other)
    {
      return m_Content == other.m_Content;
    }

  private:

    int m_Content;
  };

  class MessageSenderClass
  {
  public:
    typedef MessageSenderClass Self;

    // message without any parameters, pure notification
    Message<Self> WaveHand;

    // message without any parameters, pure notification
    Message<Self> ShowFinger;

    // message with one parameter of type std::string
    Message1<const std::string&, Self> Say;

    // message with one parameter of type double
    Message1<double, Self> WalkMeters;

    // message with one parameter of class type Package
    Message1<const Package&, Self> GivePackage;

    // message with two parameters of type int and float
    Message2<int, float, Self> ShoutAgeAndFootSize;

    void DoShowFinger() 
    { 
      ShowFinger.Send();
      // ShowFinger() does the same
    }

    void DoWaveHand() 
    { 
      WaveHand();
      // WaveHand.Send() does the same
    }

    void DoSay(const std::string& words)
    {
      Say(words);
    }

    void DoWalk(double meters)
    {
      WalkMeters(meters);
    }

    void DoGivePackage(const Package& package)
    {
      GivePackage.Send(package);
    }

    void DoShoutAgeAndFootSize(int age, float size)
    {
      ShoutAgeAndFootSize(age, size);
    }
  };


  // Receiver class remembers events received. 
  // Will tell about received events when asked.
  class MessageReceiverClass
  {
  public:

    MessageReceiverClass()
    {
      Amnesia();
    }

    void OnWaveHand()
    {
      m_HandWaved = true;
    }

    bool HandWaved()
    {
      return m_HandWaved;
    }

    void OnSay(const std::string& words)
    {
      m_WordsSaid = words;
    }

    const std::string WordsSaid()
    {
      return m_WordsSaid;
    }

    void OnWalk(double meters)
    {
      m_MetersWalked = meters;
    }

    double MetersWalked()
    {
      return m_MetersWalked;
    }

    void OnGivePackage(const Package& p)
    {
      m_PackageReceived = p;
    }

    Package PackageReceived()
    {
      return m_PackageReceived;
    }

    void OnShoutAgeAndFootSize(int age, float footSize)
    {
      m_Age = age;
      m_FootSize = footSize;
    }

    int Age()
    {
      return m_Age;
    }

    float FootSize()
    {
      return m_FootSize;
    }

    void Amnesia()
    {
      m_HandWaved = false;
      m_WordsSaid.clear();
      m_MetersWalked = 0.0;
      m_PackageReceived.Clear();
      m_Age = 0;
      m_FootSize = 0.0;
    }

    void RegisterObservers(MessageSenderClass& sender)
    {
      sender.WaveHand.AddListener( this, &MessageReceiverClass::OnWaveHand );
      sender.ShowFinger.AddListener( this, &MessageReceiverClass::OnWaveHand ); // we cannot see clearly, misinterpret this

      sender.Say.AddListener( this, &MessageReceiverClass::OnSay );
      sender.WalkMeters.AddListener( this, &MessageReceiverClass::OnWalk );
      sender.GivePackage.AddListener( this, &MessageReceiverClass::OnGivePackage );
      sender.ShoutAgeAndFootSize.AddListener( this, &MessageReceiverClass::OnShoutAgeAndFootSize );
    }

  private:

    bool m_HandWaved;
    std::string m_WordsSaid;
    double m_MetersWalked;
    Package m_PackageReceived;
    int m_Age;
    float m_FootSize;
  };

}; // end test class

} // end namespace


int mitkMessageTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("Message");

  mitk::mitkMessageTestTestClass::MessageSenderClass sender;
  mitk::mitkMessageTestTestClass::MessageReceiverClass receiver;

  MITK_TEST_CONDITION_REQUIRED(true, "Testing instantiation");

  receiver.RegisterObservers(sender);

  MITK_TEST_CONDITION_REQUIRED(true, "Testing registration to messages");

  MITK_TEST_CONDITION_REQUIRED(
    (sender.DoWaveHand(),  // This is called "comma operator". Don't ask, read!
    receiver.HandWaved()), 
    "Message without parameters");
  receiver.Amnesia();

  MITK_TEST_CONDITION_REQUIRED(
    (sender.DoShowFinger(), 
    receiver.HandWaved()), 
    "Message without parameters");
  receiver.Amnesia();

  MITK_TEST_CONDITION_REQUIRED(
    (sender.DoSay("Blooodworsch"), 
    receiver.WordsSaid() == "Blooodworsch"), 
    "Message with std::string parameter");
  receiver.Amnesia();

  MITK_TEST_CONDITION_REQUIRED(
    (sender.DoWalk(2.67), 
    (receiver.MetersWalked() - 2.67) < 0.0001 ), 
    "Message with double parameter");
  receiver.Amnesia();

  mitk::mitkMessageTestTestClass::Package package(8);
  MITK_TEST_CONDITION_REQUIRED(
    (sender.DoGivePackage(package), 
    receiver.PackageReceived() == package), 
    "Message with class parameter");
  receiver.Amnesia();

  MITK_TEST_CONDITION_REQUIRED(
    (sender.DoShoutAgeAndFootSize(46, 30.5), 
    (receiver.Age() == 46 && (receiver.FootSize() - 30.5 < 0.0001))), 
    "Message with int AND loat parameter");
  receiver.Amnesia();

  MITK_TEST_END()
}
