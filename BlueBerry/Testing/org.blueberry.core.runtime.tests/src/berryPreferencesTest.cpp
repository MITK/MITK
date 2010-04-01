/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryPreferencesTest.h"

#include <berryIBerryPreferences.h>
#include <berryIBerryPreferencesService.h>
#include <berryPlatform.h>

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

#include "berryLog.h"

#include "Poco/File.h"
#include "Poco/Path.h"

#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

namespace berry
{

  PreferencesTest::PreferencesTest(const std::string& testName)
  : berry::TestCase(testName)
  {}

  CppUnit::Test* PreferencesTest::Suite()
  {
    CppUnit::TestSuite* suite = new CppUnit::TestSuite("PreferencesTest");

    CppUnit_addTest(suite, PreferencesTest, TestAll);
    return suite;
  }

  // simple class for testing berry changed events
  class TestPreferencesChangedListener
  {
  public:
    TestPreferencesChangedListener(IBerryPreferences* _berryPrefNode) : numCalled(0), berryPrefNode(_berryPrefNode)
    {
      berryPrefNode->OnChanged.AddListener( 
        berry::MessageDelegate1<TestPreferencesChangedListener, const IBerryPreferences*> ( 
        this, &TestPreferencesChangedListener::PreferencesChanged ) 
        );
    };

    ~TestPreferencesChangedListener()
    {
      berryPrefNode->OnChanged.RemoveListener( 
        berry::MessageDelegate1<TestPreferencesChangedListener, const IBerryPreferences*> ( 
        this, &TestPreferencesChangedListener::PreferencesChanged ) 
        );
    };

    void PreferencesChanged(const IBerryPreferences*)
    {
      ++numCalled;
    }
    int numCalled;
    IBerryPreferences* berryPrefNode;
  };

  void PreferencesTest::TestAll()
  {
    IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(IPreferencesService::ID);
    assert(prefService.IsNotNull());

    /// Test for: IPreferences::GetSystemPreferences()
    IPreferences::Pointer root = prefService->GetSystemPreferences();
    assert(root.IsNotNull());

    {
      BERRY_INFO << "testing Preferences::Node(), Preferences::NodeExists(), Preferences::Parent(), " 
        "Preferences::ChildrenNames(), Preferences::RemoveNode()";

      berry::IPreferences::Pointer editorsNode(0);
      editorsNode = root->Node("/editors");
      assert(editorsNode.IsNotNull());

      assert(editorsNode->NodeExists("/editors"));

      assert(editorsNode->Parent() == root);

      berry::IPreferences::Pointer editorsGeneralNode = root->Node("/editors/general");
      assert(editorsNode->NodeExists("/editors/general")); 

      berry::IPreferences::Pointer editorsSyntaxNode = root->Node("/editors/syntax");
      assert(editorsGeneralNode->NodeExists("/editors/syntax"));

      berry::IPreferences::Pointer editorsFontNode = root->Node("/editors/font");
      assert(editorsSyntaxNode->NodeExists("/editors/font"));

      vector<string> childrenNames; 
      childrenNames.push_back("general"); 
      childrenNames.push_back("syntax"); 
      childrenNames.push_back("font");
      assert(editorsNode->ChildrenNames() == childrenNames);

      editorsFontNode->RemoveNode();
      try {
        editorsFontNode->Parent();
        failmsg("this should throw a Poco::IllegalStateException");
      }
      catch (Poco::IllegalStateException)
      {
        // expected
      }
    }

    // testing methods
    // Preferences::put*() 
    // Preferences::get*()
    {
      BERRY_INFO << "testing Preferences::put*(), Preferences::get*(), OnChanged";

      assert(root->NodeExists("/editors/general"));
      berry::IPreferences::Pointer editorsGeneralNode = root->Node("/editors/general");

      IBerryPreferences::Pointer berryEditorsGeneralNode = editorsGeneralNode.Cast< IBerryPreferences >();
      assert(berryEditorsGeneralNode.IsNotNull());

      TestPreferencesChangedListener listener(berryEditorsGeneralNode.GetPointer());

      std::string strKey = "Bad words";std::string strValue = "badword1 badword2";
      editorsGeneralNode->Put(strKey, strValue);

      assert(listener.numCalled == 1);
      assert(editorsGeneralNode->Get(strKey, "") == strValue);
      assert(editorsGeneralNode->Get("wrong key", "default value") == "default value");

      strKey = "Show Line Numbers";bool bValue = true;
      editorsGeneralNode->PutBool(strKey, bValue);
      assert(listener.numCalled == 2);
      assert(editorsGeneralNode->GetBool(strKey, !bValue) == bValue);

      strKey = "backgroundcolor"; strValue = "#00FF00";
      editorsGeneralNode->PutByteArray(strKey, strValue);
      assert(listener.numCalled == 3);
      assert(editorsGeneralNode->GetByteArray(strKey, "") == strValue);

      strKey = "update time"; double dValue = 1.23;
      editorsGeneralNode->PutDouble(strKey, dValue);
      assert(editorsGeneralNode->GetDouble(strKey, 0.0) == dValue);

      strKey = "update time float"; float fValue = 1.23f;
      editorsGeneralNode->PutFloat(strKey, fValue);
      assert(editorsGeneralNode->GetFloat(strKey, 0.0f) == fValue);

      strKey = "Break on column"; int iValue = 80;
      editorsGeneralNode->PutInt(strKey, iValue);
      assert(editorsGeneralNode->GetInt(strKey, 0) == iValue);

      strKey = "Maximum number of words"; long lValue = 11000000;
      editorsGeneralNode->PutLong(strKey, lValue);
      assert(editorsGeneralNode->GetLong(strKey, 0) == lValue);
    }

  }

}