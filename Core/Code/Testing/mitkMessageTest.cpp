/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

namespace mitk 
{

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

    // message without any parameters, pure notification
    Message<> WaveHand;

    // message without any parameters, pure notification
    Message<> ShowFinger;

    // message with one parameter of type std::string
    Message1<const std::string&> Say;

    // message with one parameter of type double
    Message1<double> WalkMeters;

    // message with one parameter of class type Package
    Message1<const Package&> GivePackage;

    // message with two parameters of type int and float
    Message2<int, float> ShoutAgeAndFootSize;

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
      sender.WaveHand += MessageDelegate<MessageReceiverClass>( this, &MessageReceiverClass::OnWaveHand );
      sender.ShowFinger += MessageDelegate<MessageReceiverClass>( this, &MessageReceiverClass::OnWaveHand ); // we cannot see clearly, misinterpret this

      sender.Say += MessageDelegate1<MessageReceiverClass, const std::string&>( this, &MessageReceiverClass::OnSay );
      sender.WalkMeters += MessageDelegate1<MessageReceiverClass, double>( this, &MessageReceiverClass::OnWalk );
      sender.GivePackage += MessageDelegate1<MessageReceiverClass, const Package&>( this, &MessageReceiverClass::OnGivePackage );
      sender.ShoutAgeAndFootSize += MessageDelegate2<MessageReceiverClass, int, float>( this, &MessageReceiverClass::OnShoutAgeAndFootSize );
    }


  private:

    bool m_HandWaved;
    std::string m_WordsSaid;
    double m_MetersWalked;
    Package m_PackageReceived;
    int m_Age;
    float m_FootSize;
};

/* MessageMacro Test classes */
class Law
 {
   private:
     std::string m_Description;

   public:

     Law(const std::string law) : m_Description(law)
     { }

     std::string GetDescription() const
     {
       return m_Description;
     }
 };

 // The NewtonMachine will issue specific events
 class NewtonMachine
 {
   mitkNewMessageMacro(AnalysisStarted);
   mitkNewMessage1Macro(AnalysisStopped, bool);
   mitkNewMessage1Macro(LawDiscovered, const Law&);
   //mitkNewMessageWithReturnMacro(PatentFiled, bool); 

   public:

     void StartAnalysis()
     {
       // send the "started" signal
       m_AnalysisStartedMessage();

       // we found a new law of nature by creating one :-)
       Law massLaw("Unit tests are mandatory!");
       m_LawDiscoveredMessage(massLaw);
     }

     void StopAnalysis()
     {
       // send the "stop" message with true, indicating
       // that an error occurred
       m_AnalysisStoppedMessage(true);
     }
     bool PatentLaw()
     {
       //bool patentAccepted = m_PatentFiledMessage();
       //return patentAccepted;
       //m_PatentFiledMessage();
       return false;
     }
 };

 class Observer
 {
   private:

     NewtonMachine* m_Machine;

   public:

     Observer(NewtonMachine* machine) : 
      m_Machine(machine), m_MachineStarted(false), m_MachineStopped(false),
      m_Error(false), m_Law("NONE")
     {
       // Add "observers", i.e. function pointers to the machine
       m_Machine->AddAnalysisStartedListener(
         ::mitk::MessageDelegate<Observer>(this, &Observer::MachineStarted));
       m_Machine->AddAnalysisStoppedListener(
         ::mitk::MessageDelegate1<Observer, bool>(this, &Observer::MachineStopped));
       m_Machine->AddLawDiscoveredListener(
         ::mitk::MessageDelegate1<Observer, const Law&>(this, &Observer::LawDiscovered));
       //m_Machine->AddPatentFiledListener(
       //  ::mitk::MessageDelegate<Observer>(this, &Observer::ReviewPatent));
      }

     ~Observer()
     {
       // Always remove your observers when finished
       m_Machine->RemoveAnalysisStartedListener(
         ::mitk::MessageDelegate<Observer>(this, &Observer::MachineStarted));
       m_Machine->RemoveAnalysisStoppedListener(
         ::mitk::MessageDelegate1<Observer, bool>(this, &Observer::MachineStopped));
       m_Machine->RemoveLawDiscoveredListener(
         ::mitk::MessageDelegate1<Observer, const Law&>(this, &Observer::LawDiscovered));
       //m_Machine->RemoveLawDiscoveredListener(
       //  ::mitk::MessageDelegate<Observer>(this, &Observer::ReviewPatent));
      }

      void MachineStarted()
      {
        m_MachineStarted = true;
      }

      void MachineStopped(bool error)
      {
        m_MachineStopped = true;
        m_Error = error;
      }

      void LawDiscovered(const Law& law)
      {
        m_Law = law;
      }
      bool ReviewPatent()
      {
        m_PatentReviewed = true;
        return false; // laws of nature are not patentable.
      }
      bool m_MachineStarted;
      bool m_MachineStopped;
      bool m_Error;
      Law m_Law;
      bool m_PatentReviewed;
  };

}; // end test class

} // end namespace


int mitkMessageTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("Message")

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
      "Message with int AND float parameter");
  receiver.Amnesia();


  mitk::mitkMessageTestTestClass::NewtonMachine newtonMachine;
  mitk::mitkMessageTestTestClass::Observer observer1(&newtonMachine);
  mitk::mitkMessageTestTestClass::Observer observer2(&newtonMachine);

  // This will send two events to registered observers
  newtonMachine.StartAnalysis();
  MITK_TEST_CONDITION(observer1.m_MachineStarted == true, "Message from Message Macro send to receiver 1");
  MITK_TEST_CONDITION(observer2.m_MachineStarted == true, "Message from Message Macro send to receiver 2");
  
  MITK_TEST_CONDITION(observer1.m_Law.GetDescription() == std::string("Unit tests are mandatory!"), 
    "Message1 from Message Macro send to receiver 1");
  MITK_TEST_CONDITION(observer2.m_Law.GetDescription() == std::string("Unit tests are mandatory!"), 
    "Message1 from Message Macro send to receiver 2");

  
  // This will send one event to registered observers
  newtonMachine.StopAnalysis();
  MITK_TEST_CONDITION(observer1.m_MachineStopped == true, "Message1 from Message Macro send to receiver 1");
  MITK_TEST_CONDITION(observer1.m_Error == true, "Message1 parameter from Message Macro send to receiver 1");

  MITK_TEST_CONDITION(observer2.m_MachineStopped == true, "Message1 from Message Macro send to receiver 2");
  MITK_TEST_CONDITION(observer2.m_Error == true, "Message1 parameter from Message Macro send to receiver 2");

  /* Message with return type tests are work in progess... */
  //bool patentSuccessful = newtonMachine.PatentLaw();   // what with return types from multiple observers?

  //MITK_TEST_CONDITION((observer1.m_PatentReviewed == true) && (patentSuccessful == false), 
  //  "Message with return type from Message Macro send to receiver 1");
  //
  //MITK_TEST_CONDITION((observer2.m_PatentReviewed == true) && (patentSuccessful == false), 
  //  "Message with return type from Message Macro send to receiver 2");
  
  MITK_TEST_END();
}
