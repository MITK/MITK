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
  MITK_TEST(RemoveFiresPropertyChanged);
  MITK_TEST(KeysWithOverrides);
  MITK_TEST(OverrideFiresEvents);
  MITK_TEST(RemoveOverrideFiresPropertyChanged);
  MITK_TEST(ClearOverridesFiresPropertyChanged);
  MITK_TEST(RemoveOverrideNoOpDoesNotFireEvents);
  MITK_TEST(FlushDoesNotPersistOverrides);

  MITK_TEST(ApplyOverrides_SingleNodeSingleProperty);
  MITK_TEST(ApplyOverrides_SingleNodeMultipleProperties);
  MITK_TEST(ApplyOverrides_NestedNodes);
  MITK_TEST(ApplyOverrides_DoNotPersist);
  MITK_TEST(ApplyOverrides_OverrideExistingValue);
  MITK_TEST(ApplyOverrides_OverrideNewKey);
  MITK_TEST(ApplyOverrides_MultipleCallsLastWins);
  MITK_TEST(ApplyOverrides_MalformedXML);
  MITK_TEST(ApplyOverrides_NullPreferences);
  MITK_TEST(ApplyOverrides_NoRootElement);
  MITK_TEST(ApplyOverrides_EmptyXML);
  MITK_TEST(ApplyOverrides_RootLevelProperties);

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

  int m_RemoveOnChangedCount = 0;
  int m_RemoveOnPropertyChangedCount = 0;

  void CountRemoveOnChanged(const mitk::IPreferences*)
  {
    ++m_RemoveOnChangedCount;
  }

  void CountRemoveOnPropertyChanged(const mitk::IPreferences::ChangeEvent&)
  {
    ++m_RemoveOnPropertyChangedCount;
  }

  void RemoveFiresPropertyChanged()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences*>(this, &mitkPreferencesTestSuite::CountRemoveOnChanged);
    preferences->OnPropertyChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences::ChangeEvent&>(this, &mitkPreferencesTestSuite::CountRemoveOnPropertyChanged);

    // Remove a persistent property
    preferences->Put("key1", "value1");
    m_RemoveOnChangedCount = 0;
    m_RemoveOnPropertyChangedCount = 0;

    preferences->Remove("key1");
    CPPUNIT_ASSERT_EQUAL(1, m_RemoveOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(1, m_RemoveOnPropertyChangedCount);

    // Force-remove an overridden key
    preferences->Put("key2", "persistent");
    preferences->Override("key2", "temporary");
    m_RemoveOnChangedCount = 0;
    m_RemoveOnPropertyChangedCount = 0;

    preferences->Remove("key2", true);
    CPPUNIT_ASSERT_EQUAL(1, m_RemoveOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(1, m_RemoveOnPropertyChangedCount);

    // Remove a non-existent key: no events
    m_RemoveOnChangedCount = 0;
    m_RemoveOnPropertyChangedCount = 0;

    preferences->Remove("nonexistent");
    CPPUNIT_ASSERT_EQUAL(0, m_RemoveOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(0, m_RemoveOnPropertyChangedCount);
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

  int m_RemoveOverrideOnChangedCount = 0;
  int m_RemoveOverrideOnPropertyChangedCount = 0;

  void CountRemoveOverrideOnChanged(const mitk::IPreferences*)
  {
    ++m_RemoveOverrideOnChangedCount;
  }

  void CountRemoveOverrideOnPropertyChanged(const mitk::IPreferences::ChangeEvent&)
  {
    ++m_RemoveOverrideOnPropertyChangedCount;
  }

  void RemoveOverrideFiresPropertyChanged()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences*>(this, &mitkPreferencesTestSuite::CountRemoveOverrideOnChanged);
    preferences->OnPropertyChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences::ChangeEvent&>(this, &mitkPreferencesTestSuite::CountRemoveOverrideOnPropertyChanged);

    preferences->Put("key", "persistent");
    preferences->Override("key", "temporary");
    m_RemoveOverrideOnChangedCount = 0;
    m_RemoveOverrideOnPropertyChangedCount = 0;

    preferences->RemoveOverride("key");
    CPPUNIT_ASSERT_EQUAL(1, m_RemoveOverrideOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(1, m_RemoveOverrideOnPropertyChangedCount);
  }

  int m_ClearOverridesOnChangedCount = 0;
  int m_ClearOverridesOnPropertyChangedCount = 0;

  void CountClearOverridesOnChanged(const mitk::IPreferences*)
  {
    ++m_ClearOverridesOnChangedCount;
  }

  void CountClearOverridesOnPropertyChanged(const mitk::IPreferences::ChangeEvent&)
  {
    ++m_ClearOverridesOnPropertyChangedCount;
  }

  void ClearOverridesFiresPropertyChanged()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences*>(this, &mitkPreferencesTestSuite::CountClearOverridesOnChanged);
    preferences->OnPropertyChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences::ChangeEvent&>(this, &mitkPreferencesTestSuite::CountClearOverridesOnPropertyChanged);

    preferences->Put("a", "1");
    preferences->Put("b", "2");
    preferences->Override("a", "x");
    preferences->Override("b", "y");
    m_ClearOverridesOnChangedCount = 0;
    m_ClearOverridesOnPropertyChangedCount = 0;

    preferences->ClearOverrides();
    CPPUNIT_ASSERT_EQUAL(1, m_ClearOverridesOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(2, m_ClearOverridesOnPropertyChangedCount);
  }

  int m_NoOpOnChangedCount = 0;
  int m_NoOpOnPropertyChangedCount = 0;

  void CountNoOpOnChanged(const mitk::IPreferences*)
  {
    ++m_NoOpOnChangedCount;
  }

  void CountNoOpOnPropertyChanged(const mitk::IPreferences::ChangeEvent&)
  {
    ++m_NoOpOnPropertyChangedCount;
  }

  void RemoveOverrideNoOpDoesNotFireEvents()
  {
    auto* preferences = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    preferences->OnChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences*>(this, &mitkPreferencesTestSuite::CountNoOpOnChanged);
    preferences->OnPropertyChanged += mitk::MessageDelegate1<mitkPreferencesTestSuite, const mitk::IPreferences::ChangeEvent&>(this, &mitkPreferencesTestSuite::CountNoOpOnPropertyChanged);

    // RemoveOverride on non-existent key should not fire
    preferences->RemoveOverride("nonexistent");
    CPPUNIT_ASSERT_EQUAL(0, m_NoOpOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(0, m_NoOpOnPropertyChangedCount);

    // ClearOverrides on empty overrides should not fire
    preferences->ClearOverrides();
    CPPUNIT_ASSERT_EQUAL(0, m_NoOpOnChangedCount);
    CPPUNIT_ASSERT_EQUAL(0, m_NoOpOnPropertyChangedCount);
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

  void ApplyOverrides_SingleNodeSingleProperty()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    prefs->Node("/org.mitk.test")->Put("color", "red");

    const std::string xml =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"color\" value=\"blue\"/>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);

    auto* node = prefs->Node("/org.mitk.test");
    CPPUNIT_ASSERT_EQUAL(std::string("blue"), node->Get("color", ""));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("color"));
  }

  void ApplyOverrides_SingleNodeMultipleProperties()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    const std::string xml =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"a\" value=\"1\"/>"
      "    <property name=\"b\" value=\"2\"/>"
      "    <property name=\"c\" value=\"3\"/>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);

    auto* node = prefs->Node("/org.mitk.test");
    CPPUNIT_ASSERT_EQUAL(std::string("1"), node->Get("a", ""));
    CPPUNIT_ASSERT_EQUAL(std::string("2"), node->Get("b", ""));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), node->Get("c", ""));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("a"));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("b"));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("c"));
  }

  void ApplyOverrides_NestedNodes()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    const std::string xml =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.views\">"
      "    <preferences name=\"editor\">"
      "      <property name=\"font\" value=\"mono\"/>"
      "    </preferences>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);

    auto* node = prefs->Node("/org.mitk.views/editor");
    CPPUNIT_ASSERT_EQUAL(std::string("mono"), node->Get("font", ""));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("font"));
  }

  void ApplyOverrides_DoNotPersist()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    auto* prefs = preferencesService->GetSystemPreferences();

    prefs->Node("/org.mitk.test")->Put("color", "red");
    prefs->Flush();

    const std::string xml =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"color\" value=\"blue\"/>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);
    CPPUNIT_ASSERT_EQUAL(std::string("blue"), prefs->Node("/org.mitk.test")->Get("color", ""));

    // Flush again — overrides must not be written to disk
    prefs->Flush();

    preferencesService->UninitializeStorage(false);
    preferencesService->InitializeStorage(m_PreferencesFilename);

    auto* reloadedNode = preferencesService->GetSystemPreferences()->Node("/org.mitk.test");
    CPPUNIT_ASSERT_EQUAL(std::string("red"), reloadedNode->Get("color", ""));
    CPPUNIT_ASSERT_EQUAL(false, reloadedNode->IsOverridden("color"));
  }

  void ApplyOverrides_OverrideExistingValue()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    prefs->Node("/org.mitk.test")->Put("size", "10");

    const std::string xml =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"size\" value=\"99\"/>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);

    auto* node = prefs->Node("/org.mitk.test");
    CPPUNIT_ASSERT_EQUAL(std::string("99"), node->Get("size", ""));

    node->RemoveOverride("size");
    CPPUNIT_ASSERT_EQUAL(std::string("10"), node->Get("size", ""));
  }

  void ApplyOverrides_OverrideNewKey()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    const std::string xml =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"newkey\" value=\"value\"/>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);

    auto* node = prefs->Node("/org.mitk.test");
    CPPUNIT_ASSERT_EQUAL(std::string("value"), node->Get("newkey", "default"));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("newkey"));

    node->RemoveOverride("newkey");
    CPPUNIT_ASSERT_EQUAL(std::string("default"), node->Get("newkey", "default"));
  }

  void ApplyOverrides_MultipleCallsLastWins()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    const std::string xml1 =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"color\" value=\"blue\"/>"
      "  </preferences>"
      "</preferences>";

    const std::string xml2 =
      "<preferences name=\"\">"
      "  <preferences name=\"org.mitk.test\">"
      "    <property name=\"color\" value=\"green\"/>"
      "  </preferences>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml1, prefs);
    mitk::ApplyPreferencesOverrides(xml2, prefs);

    auto* node = prefs->Node("/org.mitk.test");
    CPPUNIT_ASSERT_EQUAL(std::string("green"), node->Get("color", ""));
    CPPUNIT_ASSERT_EQUAL(true, node->IsOverridden("color"));
  }

  void ApplyOverrides_MalformedXML()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    CPPUNIT_ASSERT_THROW(mitk::ApplyPreferencesOverrides("this is not xml", prefs), mitk::Exception);
  }

  void ApplyOverrides_NullPreferences()
  {
    CPPUNIT_ASSERT_THROW(
      mitk::ApplyPreferencesOverrides("<preferences name=\"\"/>", nullptr),
      mitk::Exception);
  }

  void ApplyOverrides_NoRootElement()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    CPPUNIT_ASSERT_THROW(mitk::ApplyPreferencesOverrides("<?xml version=\"1.0\"?>", prefs), mitk::Exception);
  }

  void ApplyOverrides_EmptyXML()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
    CPPUNIT_ASSERT_NO_THROW(mitk::ApplyPreferencesOverrides("<preferences name=\"\"/>", prefs));
    CPPUNIT_ASSERT_EQUAL(true, prefs->ChildrenNames().empty());
  }

  void ApplyOverrides_RootLevelProperties()
  {
    auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();

    const std::string xml =
      "<preferences name=\"\">"
      "  <property name=\"globalSetting\" value=\"on\"/>"
      "</preferences>";

    mitk::ApplyPreferencesOverrides(xml, prefs);

    CPPUNIT_ASSERT_EQUAL(std::string("on"), prefs->Get("globalSetting", ""));
    CPPUNIT_ASSERT_EQUAL(true, prefs->IsOverridden("globalSetting"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPreferences)
