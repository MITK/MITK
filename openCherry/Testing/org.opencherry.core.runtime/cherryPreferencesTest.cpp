/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-02-20 18:47:40 +0100 (Fr, 20 Feb 2009) $
 Version:   $Revision: 16370 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryTestingMacros.h"
#include "cherryPreferences.h"
#include "cherryAbstractPreferencesStorage.h"

#include "mbilog.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

namespace cherry
{
  class TestPreferencesStorage: public cherry::AbstractPreferencesStorage
  {

  public:
    ///
    /// For use with cherry::SmartPtr
    ///
    cherryObjectMacro(cherry::TestPreferencesStorage)
    ///
    /// Construct a new XML-based PreferencesStorage
    ///
    TestPreferencesStorage(const Poco::File& _File)
      : AbstractPreferencesStorage(_File)
    {
      this->m_Root = new Preferences(Preferences::PropertyMap(), "", 0, this);
    }

    ///
    /// To be implemented in the subclasses.
    ///    
    virtual void Flush(IPreferences* _Preferences) throw(Poco::Exception, BackingStoreException)
    {
      std::cout << "would flush to " << this->m_File.path() << std::endl;
    }
  };
}

int cherryPreferencesTest(int /*argc*/, char* /*argv*/[])
{
  CHERRY_TEST_BEGIN("Preferences")

  cherry::IPreferences::Pointer root(0);
  cherry::TestPreferencesStorage::Pointer _PreferencesStorage(0);

  // hopefully tested
  Poco::File prefDir(Poco::Path::home()+Poco::Path::separator()+".BlueBerryTest"+Poco::Path::separator()+"prefs.xml");

  // testing methods
  // TestPreferencesStorage::TestPreferencesStorage()
  // AbstractPreferencesStorage::AbstractPreferencesStorage(),
  // AbstractPreferencesStorage::GetRoot
  {
    _PreferencesStorage = new cherry::TestPreferencesStorage(Poco::Path(prefDir.path()));
    root = _PreferencesStorage->GetRoot();
    CHERRY_TEST_CONDITION(root.IsNotNull(), "root.IsNotNull()")
  }

  // testing methods
  // Preferences::Node() 
  // Preferences::NodeExists() 
  // Preferences::Parent() 
  // Preferences::ChildrenNames() 
  // Preferences::RemoveNode() 
  {
    LOG_INFO << "testing Preferences::Node(), Preferences::NodeExists(), Preferences::Parent(), " 
      "Preferences::ChildrenNames(), Preferences::RemoveNode()";

    cherry::IPreferences::Pointer editorsNode(0);
    editorsNode = root->Node("/editors");
    CHERRY_TEST_CONDITION(editorsNode.IsNotNull(), "editorsNode.IsNotNull()")

    CHERRY_TEST_CONDITION(editorsNode->NodeExists("/editors"), "editorsNode->NodeExists(\"/editors\")") 

    CHERRY_TEST_CONDITION(editorsNode->Parent() == root, "editorsNode->Parent() == root") 

    cherry::IPreferences::Pointer editorsGeneralNode = root->Node("/editors/general");
    CHERRY_TEST_CONDITION(editorsNode->NodeExists("/editors/general"), "editorsNode->NodeExists(\"/editors/general\")") 

    cherry::IPreferences::Pointer editorsSyntaxNode = root->Node("/editors/syntax");
    CHERRY_TEST_CONDITION(editorsGeneralNode->NodeExists("/editors/syntax"), "editorsGeneralNode->NodeExists(\"/editors/syntax\")") 

    cherry::IPreferences::Pointer editorsFontNode = root->Node("/editors/font");
    CHERRY_TEST_CONDITION(editorsSyntaxNode->NodeExists("/editors/font"), "editorsSyntaxNode->NodeExists(\"/editors/font\")") 

    vector<string> childrenNames; 
    childrenNames.push_back("general"); 
    childrenNames.push_back("syntax"); 
    childrenNames.push_back("font");
    CHERRY_TEST_CONDITION(editorsNode->ChildrenNames() == childrenNames, "editorsNode->ChildrenNames() == childrenName") 

    editorsFontNode->RemoveNode();
    CHERRY_TEST_FOR_EXCEPTION(Poco::IllegalStateException, editorsFontNode->Parent());
  }

  // testing methods
  // Preferences::put*() 
  // Preferences::get*()
  {
    LOG_INFO << "testing Preferences::put*(), Preferences::get*()";
    
    CHERRY_TEST_CONDITION(root->NodeExists("/editors/general"), "root->NodeExists(\"/editors/general\")") 
    cherry::IPreferences::Pointer editorsGeneralNode = root->Node("/editors/general");

    std::string strKey = "Bad words";std::string strValue = "badword1 badword2";
    editorsGeneralNode->Put(strKey, strValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->Get(strKey, "") == strValue
      , "editorsGeneralNode->Get(strKey, \"\")")
    CHERRY_TEST_CONDITION(editorsGeneralNode->Get("wrong key", "default value") == "default value"
      , "editorsGeneralNode->Get(\"wrong key\", \"default value\") == \"default value\"")

    strKey = "Show Line Numbers";bool bValue = true;
    editorsGeneralNode->PutBool(strKey, bValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->GetBool(strKey, !bValue) == bValue
      , "editorsGeneralNode->GetBool(strKey, !bValue)")

    strKey = "backgroundcolor"; strValue = "#00FF00";
    editorsGeneralNode->PutByteArray(strKey, strValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->GetByteArray(strKey, "") == strValue
      , "editorsGeneralNode->GetByteArray(strKey, \"\")")

    strKey = "update time"; double dValue = 1.23;
    editorsGeneralNode->PutDouble(strKey, dValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->GetDouble(strKey, 0.0) == dValue
      , "editorsGeneralNode->GetDouble(strKey, 0.0)")

    strKey = "update time float"; float fValue = 1.23f;
    editorsGeneralNode->PutFloat(strKey, fValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->GetFloat(strKey, 0.0f) == fValue
      , "editorsGeneralNode->GetFloat(strKey, 0.0f)")

    strKey = "Break on column"; int iValue = 80;
    editorsGeneralNode->PutInt(strKey, iValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->GetInt(strKey, 0) == iValue
      , "editorsGeneralNode->GetInt(strKey, 0)")

    strKey = "Maximum number of words"; long lValue = 11000000;
    editorsGeneralNode->PutLong(strKey, lValue);
    CHERRY_TEST_CONDITION(editorsGeneralNode->GetLong(strKey, 0) == lValue
      , "editorsGeneralNode->GetLong(strKey, 0)")
  }

/*
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
  }*/


  CHERRY_TEST_END()
}
