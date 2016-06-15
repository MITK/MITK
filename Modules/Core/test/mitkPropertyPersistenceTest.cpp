/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPropertyPersistence.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include "mitkStringProperty.h"
#include "mitkIOMimeTypes.h"
#include <mitkNumericConstants.h>
#include <mitkEqual.h>

#include <algorithm>
#include <limits>

class mitkPropertyPersistenceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyPersistenceTestSuite);

  MITK_TEST(AddInfo);
  MITK_TEST(GetInfos);
  MITK_TEST(GetInfo);
  MITK_TEST(GetInfosByKey);
  MITK_TEST(HasInfos);
  MITK_TEST(RemoveAllInfos);
  MITK_TEST(RemoveInfos);
  MITK_TEST(RemoveInfos_withMime);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PropertyPersistenceInfo::Pointer info1;
  mitk::PropertyPersistenceInfo::Pointer info2;
  mitk::PropertyPersistenceInfo::Pointer info3;
  mitk::PropertyPersistenceInfo::Pointer info4;
  mitk::PropertyPersistenceInfo::Pointer info5;
  mitk::PropertyPersistenceInfo::Pointer info6;

  std::string prop1;
  std::string prop2;
  std::string prop3;
  std::string prop4;
  std::string prop5;
  std::string prop6;

  mitk::IPropertyPersistence* service;

  static bool checkExistance(const mitk::PropertyPersistence::InfoMapType& infos, const std::string& name, const mitk::PropertyPersistenceInfo* info)
  {
    auto infoRange = infos.equal_range(name);

    auto predicate = [info](const std::pair<const std::string, mitk::PropertyPersistenceInfo::Pointer>& x){return infosAreEqual(info, x.second); };

    auto finding = std::find_if(infoRange.first, infoRange.second, predicate);

    bool result = finding != infoRange.second;
    return result;
  }

  static bool infosAreEqual(const mitk::PropertyPersistenceInfo* ref, const mitk::PropertyPersistenceInfo* info)
  {
    bool result = true;

    if (!info || !ref)
    {
      return false;
    }

    result = result && ref->GetKey() == info->GetKey();
    result = result && ref->GetMimeTypeName() == info->GetMimeTypeName();
    return result;
  }

public:

  void setUp() override
  {
    service = mitk::CreateTestInstancePropertyPersistence();

    info1 = mitk::PropertyPersistenceInfo::New("key1");

    info2 = mitk::PropertyPersistenceInfo::New("key2", "mime2");
    info3 = mitk::PropertyPersistenceInfo::New("key3", "mime3");
    info4 = mitk::PropertyPersistenceInfo::New("key2", "mime2");
    info5 = mitk::PropertyPersistenceInfo::New("key5", "mime5");

    prop1 = "prop1";
    prop2 = "prop1";
    prop3 = "prop1";
    prop4 = "prop4";
    prop5 = "prop5";

    service->AddInfo(prop1, info1, false);
    service->AddInfo(prop2, info2, false);
    service->AddInfo(prop3, info3, false);
    service->AddInfo(prop4, info4, false);
    service->AddInfo(prop5, info5, false);
  }

  void tearDown() override
  {
    delete service;
  }

  void AddInfo()
  {
    mitk::PropertyPersistenceInfo::Pointer info2_new = mitk::PropertyPersistenceInfo::New("newKey", "otherMime");
    mitk::PropertyPersistenceInfo::Pointer info2_otherKey = mitk::PropertyPersistenceInfo::New("otherKey", "mime2");

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (no overwrite) -> no adding", !service->AddInfo(prop2, info2_otherKey, false));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (no overwrite) -> no adding -> key should not be changed.", service->GetInfo(prop2, "mime2", false)->GetKey() == "key2");

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (overwrite) -> adding", service->AddInfo(prop2, info2_otherKey, true));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (no overwrite) -> adding -> key should be changed.", service->GetInfo(prop2, "mime2", false)->GetKey() == "otherKey");

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (other mime type; no overwrite) -> adding", service->AddInfo(prop2, info2_new, false));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (other mime type; no overwrite) -> adding -> info exists.", service->GetInfo(prop2, "otherMime", false).IsNotNull());

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (new prop name; no overwrite) -> adding", service->AddInfo("newProp", info2_new, false));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (new prop name; no overwrite) -> adding ->info exists.", service->GetInfo("newProp", "otherMime", false).IsNotNull());
  }

  void GetInfos()
  {
    mitk::PropertyPersistence::InfoMapType infos = service->GetInfos(prop1);
    CPPUNIT_ASSERT(infos.size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, prop1, info1));
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, prop2, info2));
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, prop3, info3));

    infos = service->GetInfos(prop4);
    CPPUNIT_ASSERT(infos.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, prop4, info4));

    infos = service->GetInfos("unkown");
    CPPUNIT_ASSERT_MESSAGE("Check size of result for unkown prop.", infos.empty());
  }

  void GetInfosByKey()
  {
    mitk::PropertyPersistence::InfoMapType infos = service->GetInfosByKey("key2");
    CPPUNIT_ASSERT(infos.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, prop2, info2));
    CPPUNIT_ASSERT_MESSAGE("Check expected element 2.", checkExistance(infos, prop4, info4));

    infos = service->GetInfosByKey("key5");
    CPPUNIT_ASSERT(infos.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, prop5, info5));

    infos = service->GetInfosByKey("unkownkey");
    CPPUNIT_ASSERT_MESSAGE("Check size of result for unkown key.", infos.empty());
  }

  void GetInfo()
  {
    mitk::PropertyPersistenceInfo::Pointer foundInfo = service->GetInfo(prop1, "mime2", false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfo (existing element, no wildcard allowed, wildcard exists).", infosAreEqual(info2, foundInfo));
    foundInfo = service->GetInfo(prop1, "mime2", true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfo (existing element, wildcard allowed, wildcard exists).", infosAreEqual(info2, foundInfo));
    foundInfo = service->GetInfo(prop1, "unknownmime", false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfo (inexisting element, no wildcard allowed, wildcard exists).", foundInfo.IsNull());
    foundInfo = service->GetInfo(prop1, "unknownmime", true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfo (inexisting element, wildcard allowed, wildcard exists).", infosAreEqual(info1, foundInfo));

    foundInfo = service->GetInfo(prop4, "unknownmime", false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfo (inexisting element, no wildcard allowed).", foundInfo.IsNull());
    foundInfo = service->GetInfo(prop4, "unknownmime", true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfo (inexisting element, wildcard allowed).", foundInfo.IsNull());
  }

  void HasInfos()
  {
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop1)", service->HasInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (unkown prop)", !service->HasInfos("unkownProp"));
  }

  void RemoveAllInfos()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveAllInfos());
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop1)", !service->HasInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", !service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", !service->HasInfos(prop5));
  }


  void RemoveInfos()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop1)", !service->HasInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", !service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop5));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", !service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos("unknown_prop"));
  }

  void RemoveInfos_withMime()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop1, "mime2"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos if info was removed",service->GetInfo(prop1, "mime2", false).IsNull());
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos, if other info of same property name still exists", service->GetInfo(prop1, "mime3", false).IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos, if other info of other property name but same mime still exists", service->GetInfo(prop4, "mime2", false).IsNotNull());

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop5, "wrongMime"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos on prop 5 with wrong mime", service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop5, "mime5"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos on prop 5", !service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos("unkown_prop", "mime2"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos, if unkown property name but exting mime was used", service->HasInfos(prop4));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyPersistence)
