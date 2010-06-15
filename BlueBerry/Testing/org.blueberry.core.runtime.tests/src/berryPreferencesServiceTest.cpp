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
#include <berryIBerryPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatform.h>

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
    try
    {    
      IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(IPreferencesService::ID);
      assert(prefService.IsNotNull());

      /// Test for: IPreferences::GetSystemPreferences()
      IPreferences::Pointer sysPrefs = prefService->GetSystemPreferences();
      assert(sysPrefs.IsNotNull());

      /// Test for: IPreferences::GetUserPreferences(std::string name)
      IPreferences::Pointer testUserPrefs = prefService->GetUserPreferences("testUser");
      assert(testUserPrefs.IsNotNull());

      /// Test for: IPreferences::GetUsers()
      std::vector<std::string> userList = prefService->GetUsers();
      // userList should now contain "testUser"
      bool userListContainsTestUser = false;
      for (std::vector<std::string>::iterator it = userList.begin()
        ; it != userList.end(); it++)
      {
        if(*it == "testUser")
        {
          userListContainsTestUser = true;
          break;
        }
      }
      assert(userListContainsTestUser);

      IBerryPreferencesService::Pointer berryPrefService = prefService.Cast<IBerryPreferencesService>();    
      // optional test for IBerryPreferencesService
      if(berryPrefService.IsNotNull())
      {
        /// Test for: IBerryPreferencesService::ExportPreferences(Poco::File f, std::string name="")

        // write general prefs
        std::string sysPrefsExportFilePath = Poco::Path::temp() + Poco::Path::separator() + "systemBerryPreferences";
        Poco::File sysPrefsExportFile(sysPrefsExportFilePath);
        sysPrefs->PutInt("testNumber", 1);
        berryPrefService->ExportPreferences(sysPrefsExportFile);
        // assert somethings was written
        assert(sysPrefsExportFile.getSize() > 0);

        // write testUser prefs
        std::string testUserPrefsExportFilePath = Poco::Path::temp() + Poco::Path::separator() + "testUserBerryPreferences";
        Poco::File testUserPrefsExportFile(testUserPrefsExportFilePath);
        testUserPrefs->PutInt("testNumber", 2);
        berryPrefService->ExportPreferences(testUserPrefsExportFile, "testUser");
        assert(testUserPrefsExportFile.getSize() > 0);

        /// Test for: IBerryPreferencesService::ImportPreferences(Poco::File f, std::string name="")

        // import general prefs
        // change testNumber value
        sysPrefs->PutInt("testNumber", 3);
        berryPrefService->ImportPreferences(sysPrefsExportFile);
        // "testNumber" preference should now again be overwritten with its old value 1
        assert(sysPrefs->GetInt("testNumber", 3) == 1);

        // import testUser prefs
        // change testNumber value
        testUserPrefs->PutInt("testNumber", 4);
        berryPrefService->ImportPreferences(testUserPrefsExportFile, "testUser");
        // "testNumber" preference should now again be overwritten with its old value 2
        assert(testUserPrefs->GetInt("testNumber", 4) == 2);
        
        // delete files again
        sysPrefsExportFile.remove();
        testUserPrefsExportFile.remove();
      }
    }
    catch (Poco::CreateFileException& e)
    {
      std::string msg = "Failed to create preferences file: ";
      msg.append(e.what());
      this->fail( msg );
    }
    catch (std::exception& e)
    {
      this->fail( e.what() );
    }
    catch (...)
    {
      this->fail( "unknown exception occured" );
    }
  }
}