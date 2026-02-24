/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkIOUtil.h>

class mitkPreferencesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPreferencesTestSuite);

  MITK_TEST(Node);
  MITK_TEST(OnChanged);
  MITK_TEST(OnPropertyChanged);
  MITK_TEST(GetInt);
  MITK_TEST(GetFloat);
  MITK_TEST(OverrideString);
  MITK_TEST(OverrideInt);
  MITK_TEST(OverrideBool);
  MITK_TEST(OverrideNonExistentKey);
  MITK_TEST(IsOverridden);
  MITK_TEST(RemoveOverride);
  MITK_TEST(ClearOverrides);
  MITK_TEST(ClearIncludingOverrides);
  MITK_TEST(RemoveWithOverride);
  MITK_TEST(KeysWithOverrides);
  MITK_TEST(OverrideFiresEvents);
  MITK_TEST(FlushDoesNotPersistOverrides);

  CPPUNIT_TEST_SUITE_END();

  int m_NumberOfOnChangedEvents = 0;
  int m_NumberOfOnPropertyChangedEvents = 0;
  std::string m_PreferencesFilename;

public:

  void setUp() override
  {
    m_PreferencesFilename = mitk::IOUtil::CreateTemporaryFile("prefs_XXXXXX.xml");
    fs::remove(m_PreferencesFilename); // We need a temporary filename, not an actual file

    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    preferencesService->InitializeStorage(m_PreferencesFilename);
  }

  void tearDown() override
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    preferencesService->UninitializeStorage();
  }

  void Node()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    // a > aa
    // b > bb > bbb
    // b > bb2

    auto* aPrefs = preferences->Node("a");
    aPrefs->Node("aa");
    auto* bPrefs = aPrefs->Node("/b");
    bPrefs->Node("bb/bbb");
    preferences->Node("/b/bb2");
    auto* bbPrefs = bPrefs->Node("bb");

    auto nodeNames = preferences->ChildrenNames();

    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(nodeNames.size()));
    CPPUNIT_ASSERT(std::find(nodeNames.begin(), nodeNames.end(), "a") != nodeNames.end());
    CPPUNIT_ASSERT(std::find(nodeNames.begin(), nodeNames.end(), "b") != nodeNames.end());

    nodeNames = aPrefs->ChildrenNames();

    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(nodeNames.size()));
    CPPUNIT_ASSERT(std::find(nodeNames.begin(), nodeNames.end(), "aa") != nodeNames.end());

    nodeNames = bPrefs->ChildrenNames();

    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(nodeNames.size()));
    CPPUNIT_ASSERT(std::find(nodeNames.begin(), nodeNames.end(), "bb") != nodeNames.end());
    CPPUNIT_ASSERT(std::find(nodeNames.begin(), nodeNames.end(), "bb2") != nodeNames.end());

    nodeNames = bbPrefs->ChildrenNames();

    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(nodeNames.size()));
    CPPUNIT_ASSERT(std::find(nodeNames.begin(), nodeNames.end(), "bbb") != nodeNames.end());
    CPPUNIT_ASSERT_EQUAL(std::string("/b/bb/bbb"), bbPrefs->Node("bbb")->AbsolutePath());
  }

  void CountOnChangedEvents(const mitk::IPreferences*)
  {
    ++m_NumberOfOnChangedEvents;
  }

  void OnChanged()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences*>(this, &mitkPreferencesTestSuite::CountOnChangedEvents);

    CPPUNIT_ASSERT_EQUAL(0, m_NumberOfOnChangedEvents);

    preferences->Node("a");
    CPPUNIT_ASSERT_EQUAL(1, m_NumberOfOnChangedEvents);

    preferences->Node("a");
    CPPUNIT_ASSERT_EQUAL(1, m_NumberOfOnChangedEvents);

    preferences->Node("a/aa/aaa");
    CPPUNIT_ASSERT_EQUAL(1, m_NumberOfOnChangedEvents);

    preferences->Node("b/bb");
    CPPUNIT_ASSERT_EQUAL(2, m_NumberOfOnChangedEvents);

    preferences->Node("b");
    CPPUNIT_ASSERT_EQUAL(2, m_NumberOfOnChangedEvents);
  }

  void CountOnPropertyChangedEvents(const mitk::IPreferences::ChangeEvent& e)
  {
    ++m_NumberOfOnPropertyChangedEvents;

    CPPUNIT_ASSERT_EQUAL(std::string("pref"), e.GetProperty());
    CPPUNIT_ASSERT_EQUAL(std::string(""), e.GetOldValue());
    CPPUNIT_ASSERT_EQUAL(std::string("something"), e.GetNewValue());
  }

  void OnPropertyChanged()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnPropertyChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences::ChangeEvent&>(this, &mitkPreferencesTestSuite::CountOnPropertyChangedEvents);

    CPPUNIT_ASSERT_EQUAL(0, m_NumberOfOnPropertyChangedEvents);

    preferences->Put("pref", "");
    CPPUNIT_ASSERT_EQUAL(0, m_NumberOfOnPropertyChangedEvents);

    preferences->Put("pref", "something");
    CPPUNIT_ASSERT_EQUAL(1, m_NumberOfOnPropertyChangedEvents);
  }

  void GetInt()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    const int expectedValue = 42;

    preferences->PutInt("integer", expectedValue);
    CPPUNIT_ASSERT_EQUAL(expectedValue, preferences->GetInt("integer", 0));

    preferences->Put("overflow", "4200000000");
    CPPUNIT_ASSERT_THROW(preferences->GetInt("overflow", 0), mitk::Exception);

    preferences->Put("string", "fourty two");    
    CPPUNIT_ASSERT_THROW(preferences->GetInt("string", 0), mitk::Exception);
  }

  void GetFloat()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    float expectedValue = 3.14f;

    preferences->PutFloat("float", expectedValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedValue, preferences->GetFloat("float", 0.0f), mitk::eps);

    preferences->Put("overflow", "3.14e100");
    CPPUNIT_ASSERT_THROW(preferences->GetFloat("overflow", 0.0f), mitk::Exception);

    preferences->Put("string", "pi");
    CPPUNIT_ASSERT_THROW(preferences->GetFloat("string", 0.0f), mitk::Exception);
  }

  void OverrideString()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("color", "red");
    CPPUNIT_ASSERT_EQUAL(std::string("red"), preferences->Get("color", ""));

    preferences->Override("color", "blue");
    CPPUNIT_ASSERT_EQUAL(std::string("blue"), preferences->Get("color", ""));

    preferences->RemoveOverride("color");
    CPPUNIT_ASSERT_EQUAL(std::string("red"), preferences->Get("color", ""));
  }

  void OverrideInt()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->PutInt("count", 10);
    CPPUNIT_ASSERT_EQUAL(10, preferences->GetInt("count", 0));

    preferences->OverrideInt("count", 99);
    CPPUNIT_ASSERT_EQUAL(99, preferences->GetInt("count", 0));
  }

  void OverrideBool()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->PutBool("enabled", false);
    CPPUNIT_ASSERT_EQUAL(false, preferences->GetBool("enabled", true));

    preferences->OverrideBool("enabled", true);
    CPPUNIT_ASSERT_EQUAL(true, preferences->GetBool("enabled", false));
  }

  void OverrideNonExistentKey()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    CPPUNIT_ASSERT_NO_THROW(preferences->Override("newkey", "value"));
    CPPUNIT_ASSERT_EQUAL(std::string("value"), preferences->Get("newkey", ""));
    CPPUNIT_ASSERT_EQUAL(true, preferences->IsOverridden("newkey"));
  }

  void IsOverridden()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("key", "value");
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("key"));

    preferences->Override("key", "override");
    CPPUNIT_ASSERT_EQUAL(true, preferences->IsOverridden("key"));

    preferences->RemoveOverride("key");
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("key"));
  }

  void RemoveOverride()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("key", "persistent");
    preferences->Override("key", "temporary");
    CPPUNIT_ASSERT_EQUAL(std::string("temporary"), preferences->Get("key", ""));

    preferences->RemoveOverride("key");
    CPPUNIT_ASSERT_EQUAL(std::string("persistent"), preferences->Get("key", ""));
  }

  void ClearOverrides()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("a", "1");
    preferences->Put("b", "2");
    preferences->Override("a", "x");
    preferences->Override("b", "y");
    CPPUNIT_ASSERT_EQUAL(true, preferences->IsOverridden("a"));
    CPPUNIT_ASSERT_EQUAL(true, preferences->IsOverridden("b"));

    preferences->ClearOverrides();
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("a"));
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("b"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), preferences->Get("a", ""));
    CPPUNIT_ASSERT_EQUAL(std::string("2"), preferences->Get("b", ""));
  }

  void ClearIncludingOverrides()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("a", "1");
    preferences->Override("a", "x");
    preferences->Override("b", "y");

    preferences->Clear(true);

    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("a"));
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("b"));
    CPPUNIT_ASSERT_EQUAL(std::string("default"), preferences->Get("a", "default"));
    CPPUNIT_ASSERT_EQUAL(std::string("default"), preferences->Get("b", "default"));
  }

  void RemoveWithOverride()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("key", "persistent");
    preferences->Override("key", "temporary");

    CPPUNIT_ASSERT_EQUAL(false, preferences->Remove("key"));
    CPPUNIT_ASSERT_EQUAL(std::string("temporary"), preferences->Get("key", ""));

    CPPUNIT_ASSERT_EQUAL(true, preferences->Remove("key", true));
    CPPUNIT_ASSERT_EQUAL(std::string("default"), preferences->Get("key", "default"));
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("key"));
  }

  void KeysWithOverrides()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    preferences->Put("a", "1");
    preferences->Put("b", "2");
    preferences->Override("a", "x");

    auto keys = preferences->Keys();
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(keys.size()));

    auto keysWithOverrides = preferences->Keys(true);
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(keysWithOverrides.size()));
  }

  int m_OverrideOnChangedCount = 0;
  int m_OverrideOnPropertyChangedCount = 0;

  void CountOverrideOnChanged(const mitk::IPreferences*)
  {
    ++m_OverrideOnChangedCount;
  }

  void CountOverrideOnPropertyChanged(const mitk::IPreferences::ChangeEvent&)
  {
    ++m_OverrideOnPropertyChangedCount;
  }

  void OverrideFiresEvents()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences*>(this, &mitkPreferencesTestSuite::CountOverrideOnChanged);
    preferences->OnPropertyChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences::ChangeEvent&>(this, &mitkPreferencesTestSuite::CountOverrideOnPropertyChanged);

    preferences->Put("key", "value");
    CPPUNIT_ASSERT_EQUAL(1, m_OverrideOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(1, m_OverrideOnPropertyChangedCount);

    preferences->Override("key", "override");
    CPPUNIT_ASSERT_EQUAL(2, m_OverrideOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(2, m_OverrideOnPropertyChangedCount);
  }

  void FlushDoesNotPersistOverrides()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    auto* preferences = preferencesService->GetSystemPreferences();

    preferences->Put("key", "persistent");
    preferences->Override("key", "temporary");
    CPPUNIT_ASSERT_EQUAL(std::string("temporary"), preferences->Get("key", ""));

    preferences->Flush();

    // Re-initialize storage to reload from disk
    preferencesService->UninitializeStorage(false);
    preferencesService->InitializeStorage(m_PreferencesFilename);

    preferences = preferencesService->GetSystemPreferences();
    CPPUNIT_ASSERT_EQUAL(std::string("persistent"), preferences->Get("key", ""));
    CPPUNIT_ASSERT_EQUAL(false, preferences->IsOverridden("key"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPreferences)
