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
#include "cherryPreferencesService.h"

#include "mbilog.h"

#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/AutoPtr.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

int cherryPreferencesServiceTest(int /*argc*/, char* /*argv*/[])
{
  CHERRY_TEST_BEGIN("PreferencesService")
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

    //std::string defaultPrefDir = cherry::PreferencesService::GetDefaultPreferencesDirPath();
    std::string defaultPrefDir = "testPrefDir";
    Poco::File prefDir("testPrefDir");

    // tidy up from prior tests
    if(prefDir.exists())
      prefDir.remove();
    {
	    cherry::PreferencesService::Pointer prefService(new cherry::PreferencesService(prefDir.path()));
  	
	    // hopefully tested
	    std::string defaultPrefFileName = cherry::PreferencesService::GetDefaultPreferencesFileName();
  	  	
	    //# remove old user and sys files (if their was a previous test
      ostringstream s; s << defaultPrefDir << Poco::Path::separator() << defaultPrefFileName;
	    sysPrefFile = s.str();
	    s.str(""); s << defaultPrefDir << Poco::Path::separator() << userName1 << defaultPrefFileName;
	    user1PrefFile = s.str();
	    s.str(""); s << defaultPrefDir << Poco::Path::separator() << userName2 << defaultPrefFileName;
	    user2PrefFile = s.str();

      //# test GetSystemPreferences
      cherry::IPreferences::Pointer sysPrefsRoot = prefService->GetSystemPreferences();
      CHERRY_TEST_CONDITION(sysPrefsRoot.IsNotNull(), "sysPrefsRoot.IsNotNull()")
      cherry::IPreferences::Pointer sysPrefsGeneral = sysPrefsRoot->Node("/general");
      sysPrefsGeneral->Put("font-size", "10");

      //# test GetUserPreferences
      cherry::IPreferences::Pointer user1PrefsRoot = prefService->GetUserPreferences(userName1);
      CHERRY_TEST_CONDITION(user1PrefsRoot.IsNotNull(), "user1PrefsRoot.IsNotNull()")
      cherry::IPreferences::Pointer user1PrefsGeneral = user1PrefsRoot->Node("/general");
      user1PrefsGeneral->PutInt("font-size", 10);

      //# test GetUserPreferences
      cherry::IPreferences::Pointer user2PrefsRoot = prefService->GetUserPreferences(userName2);
      CHERRY_TEST_CONDITION(user2PrefsRoot.IsNotNull(), "user2PrefsRoot.IsNotNull()")
      cherry::IPreferences::Pointer user2PrefsGeneral = user2PrefsRoot->Node("/general");
      user2PrefsGeneral->PutFloat("font-size", 10.32324f);

      //# test GetUsers
      vector<string> usersVector = prefService->GetUsers();
      CHERRY_TEST_CONDITION(usersVector == validUsersVector, "usersVector == validUsersVector")
    }
    // pref service was destructed files should exist now
    CHERRY_TEST_CONDITION(sysPrefFile.exists(), "sysPrefFile.exists()")
    CHERRY_TEST_CONDITION(user1PrefFile.exists(), "user1PrefFile.exists()")
    CHERRY_TEST_CONDITION(user2PrefFile.exists(), "user2PrefFile.exists()")

    // now create a new pref service that reads the files in again
    cherry::PreferencesService::Pointer prefService(new cherry::PreferencesService());

    //# test GetUsers
    vector<string> usersVector = prefService->GetUsers();
    CHERRY_TEST_CONDITION(usersVector == validUsersVector, "usersVector == validUsersVector")

    //# test GetSystemPreferences
    cherry::IPreferences::Pointer sysPrefsRoot = prefService->GetSystemPreferences();
    CHERRY_TEST_CONDITION(sysPrefsRoot.IsNotNull(), "sysPrefsRoot.IsNotNull()")
    cherry::IPreferences::Pointer sysPrefsGeneral = sysPrefsRoot->Node("/general");
    CHERRY_TEST_CONDITION(sysPrefsGeneral->Get("font-size", "") == "10", "sysPrefsGeneral->Get(\"font-size\", \"\") == \"10\"")

    //# test GetUserPreferences
    cherry::IPreferences::Pointer user1PrefsRoot = prefService->GetUserPreferences(userName1);
    CHERRY_TEST_CONDITION(user1PrefsRoot.IsNotNull(), "user1PrefsRoot.IsNotNull()")
    cherry::IPreferences::Pointer user1PrefsGeneral = user1PrefsRoot->Node("/general");
    CHERRY_TEST_CONDITION(user1PrefsGeneral->GetInt("font-size", 0) == 10, "user1PrefsGeneral->GetInt(\"font-size\", 0)")

    //# test GetUserPreferences
    cherry::IPreferences::Pointer user2PrefsRoot = prefService->GetUserPreferences(userName2);
    CHERRY_TEST_CONDITION(user2PrefsRoot.IsNotNull(), "user2PrefsRoot.IsNotNull()")
    cherry::IPreferences::Pointer user2PrefsGeneral = user2PrefsRoot->Node("/general");
    float user2FontSize = user2PrefsGeneral->GetFloat("font-size", 0.0f);
    CHERRY_TEST_CONDITION(user2FontSize == 10.32324f, "user2PrefsGeneral->GetFloat(\"font-size\", 0.0f) == 10.32324f")

    if(prefDir.exists())
      prefDir.remove();

  }
  catch (Poco::Exception& e)
  {
    LOG_WARN("PreferencesService") << e.message();
  }

  CHERRY_TEST_END()
}
