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

  CPPUNIT_TEST_SUITE_END();

  int m_NumberOfOnChangedEvents = 0;
  int m_NumberOfOnPropertyChangedEvents = 0;

public:

  void setUp() override
  {
    const auto filename = mitk::IOUtil::CreateTemporaryFile("prefs_XXXXXX.xml");
    std::filesystem::remove(filename); // We need a temporary filename, not an actual file

    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    preferencesService->InitializeStorage(filename);
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
};

MITK_TEST_SUITE_REGISTRATION(mitkPreferences)
