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

#include "berryPreferencesServiceTest.h"
#include <internal/berryPreferencesService.h>

#include "berryLog.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/AutoPtr.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

namespace berry
{

  PreferencesServiceTest::PreferencesServiceTest(const std::string& testName)
  : berry::TestCase(testName)
  {}

  CppUnit::Test* PreferencesServiceTest::Suite()
  {
    CppUnit::TestSuite* suite = new CppUnit::TestSuite("PreferencesServiceTest");

    CppUnit_addTest(suite, PreferencesServiceTest, TestAll);
    return suite;
  }


  void PreferencesServiceTest::TestAll()
  {

    Poco::File sysPrefFile;
    Poco::File user1PrefFile;
    Poco::File user2PrefFile;

    std::string userName1 = "testUser1";
    std::string userName2 = "testUser2";



    vector<string> validUsersVector; 
    validUsersVector.push_back(""); 
    validUsersVector.push_back(userName1); 
    validUsersVector.push_back(userName2);

    try
    {

      //std::string defaultPrefDir = berry::PreferencesService::GetDefaultPreferencesDirPath();
      std::string defaultPrefDir = berry::PreferencesService::GetDefaultPreferencesDirPath();
      Poco::File prefDir(defaultPrefDir);

      // tidy up from prior tests
      if(prefDir.exists())
        prefDir.remove(true);
      {
	      berry::PreferencesService::Pointer prefService(new berry::PreferencesService(prefDir.path()));
    	
	      // hopefully tested
	      std::string defaultPrefFileName = berry::PreferencesService::GetDefaultPreferencesFileName();
    	  	
	      //# remove old user and sys files (if their was a previous test
        ostringstream s; s << defaultPrefDir << Poco::Path::separator() << defaultPrefFileName;
	      sysPrefFile = s.str();
	      s.str(""); s << defaultPrefDir << Poco::Path::separator() << userName1 << defaultPrefFileName;
	      user1PrefFile = s.str();
	      s.str(""); s << defaultPrefDir << Poco::Path::separator() << userName2 << defaultPrefFileName;
	      user2PrefFile = s.str();

        //# test GetSystemPreferences
        berry::IPreferences::Pointer sysPrefsRoot = prefService->GetSystemPreferences();
        assert(sysPrefsRoot.IsNotNull());
        berry::IPreferences::Pointer sysPrefsGeneral = sysPrefsRoot->Node("/general");
        sysPrefsGeneral->Put("font-size", "10");

        //# test GetUserPreferences
        berry::IPreferences::Pointer user1PrefsRoot = prefService->GetUserPreferences(userName1);
        assert(user1PrefsRoot.IsNotNull());
        berry::IPreferences::Pointer user1PrefsGeneral = user1PrefsRoot->Node("/general");
        user1PrefsGeneral->PutInt("font-size", 10);

        //# test GetUserPreferences
        berry::IPreferences::Pointer user2PrefsRoot = prefService->GetUserPreferences(userName2);
        assert(user2PrefsRoot.IsNotNull());
        berry::IPreferences::Pointer user2PrefsGeneral = user2PrefsRoot->Node("/general");
        user2PrefsGeneral->PutFloat("font-size", 10.32324f);

        //# test GetUsers
        vector<string> usersVector = prefService->GetUsers();
        assert(usersVector == validUsersVector);
      }
      // pref service was destructed files should exist now
      assert(sysPrefFile.exists());
      assert(user1PrefFile.exists());
      assert(user2PrefFile.exists());

      // now create a new pref service that reads the files in again
      berry::PreferencesService::Pointer prefService(new berry::PreferencesService());

      //# test GetUsers
      vector<string> usersVector = prefService->GetUsers();
      //assert(usersVector == validUsersVector);

      //# test GetSystemPreferences
      berry::IPreferences::Pointer sysPrefsRoot = prefService->GetSystemPreferences();
      assert(sysPrefsRoot.IsNotNull());
      berry::IPreferences::Pointer sysPrefsGeneral = sysPrefsRoot->Node("/general");
      assert(sysPrefsGeneral->Get("font-size", "") == "10");

      //# test GetUserPreferences
      berry::IPreferences::Pointer user1PrefsRoot = prefService->GetUserPreferences(userName1);
      assert(user1PrefsRoot.IsNotNull());
      berry::IPreferences::Pointer user1PrefsGeneral = user1PrefsRoot->Node("/general");
      assert(user1PrefsGeneral->GetInt("font-size", 0) == 10);

      //# test GetUserPreferences
      berry::IPreferences::Pointer user2PrefsRoot = prefService->GetUserPreferences(userName2);
      assert(user2PrefsRoot.IsNotNull());
      berry::IPreferences::Pointer user2PrefsGeneral = user2PrefsRoot->Node("/general");
      float user2FontSize = user2PrefsGeneral->GetFloat("font-size", 0.0f);
      assert(user2FontSize == 10.32324f);

      if(prefDir.exists())
        prefDir.remove(true);

    }
    catch (Poco::Exception& e)
    {
      failmsg(e.message());
    }

  }
}