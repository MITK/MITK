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

    MITK_TEST_CONDITION_REQUIRED(!service->AddInfo(prop2, info2_otherKey, false), "Testing addinfo of already existing info (no overwrite) -> no adding");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo(prop2, "mime2", false)->GetKey() == "key2", "Testing addinfo of already existing info (no overwrite) -> no adding -> key should not be changed.");

    MITK_TEST_CONDITION_REQUIRED(service->AddInfo(prop2, info2_otherKey, true), "Testing addinfo of already existing info (overwrite) -> adding");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo(prop2, "mime2", false)->GetKey() == "otherKey", "Testing addinfo of already existing info (no overwrite) -> adding -> key should be changed.");

    MITK_TEST_CONDITION_REQUIRED(service->AddInfo(prop2, info2_new, false), "Testing addinfo of info (other mime type; no overwrite) -> adding");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo(prop2, "otherMime", false).IsNotNull(), "Testing addinfo of info (other mime type; no overwrite) -> adding -> info exists.");

    MITK_TEST_CONDITION_REQUIRED(service->AddInfo("newProp", info2_new, false), "Testing addinfo of info (new prop name; no overwrite) -> adding");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo("newProp", "otherMime", false).IsNotNull(), "Testing addinfo of info (new prop name; no overwrite) -> adding ->info exists.");
  }

  void GetInfos()
  {
    mitk::PropertyPersistence::InfoMapType infos = service->GetInfos(prop1);
    MITK_TEST_EQUAL(infos.size(), 3, "Check size of result for Prop1");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop1, info1), "Check expected element 1.");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop2, info2), "Check expected element 2.");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop3, info3), "Check expected element 3.");

    infos = service->GetInfos(prop4);
    MITK_TEST_EQUAL(infos.size(), 1, "Check size of result for Prop1");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop4, info4), "Check expected element 1.");

    infos = service->GetInfos("unkown");
    MITK_TEST_CONDITION_REQUIRED(infos.empty(), "Check size of result for unkown prop.");
  }

  void GetInfosByKey()
  {
    mitk::PropertyPersistence::InfoMapType infos = service->GetInfosByKey("key2");
    MITK_TEST_EQUAL(infos.size(), 2, "Check size of result for key2");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop2, info2), "Check expected element 1.");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop4, info4), "Check expected element 2.");

    infos = service->GetInfosByKey("key5");
    MITK_TEST_EQUAL(infos.size(), 1, "Check size of result for Prop1");
    MITK_TEST_CONDITION_REQUIRED(checkExistance(infos, prop5, info5), "Check expected element 1.");

    infos = service->GetInfosByKey("unkownkey");
    MITK_TEST_CONDITION_REQUIRED(infos.empty(), "Check size of result for unkown key.");
  }

  void GetInfo()
  {
    mitk::PropertyPersistenceInfo::Pointer foundInfo = service->GetInfo(prop1, "mime2", false);
    MITK_TEST_CONDITION_REQUIRED(infosAreEqual(info2, foundInfo), "Check GetInfo (existing element, no wildcard allowed, wildcard exists).");
    foundInfo = service->GetInfo(prop1, "mime2", true);
    MITK_TEST_CONDITION_REQUIRED(infosAreEqual(info2, foundInfo), "Check GetInfo (existing element, wildcard allowed, wildcard exists).");
    foundInfo = service->GetInfo(prop1, "unknownmime", false);
    MITK_TEST_CONDITION_REQUIRED(foundInfo.IsNull(), "Check GetInfo (inexisting element, no wildcard allowed, wildcard exists).");
    foundInfo = service->GetInfo(prop1, "unknownmime", true);
    MITK_TEST_CONDITION_REQUIRED(infosAreEqual(info1, foundInfo), "Check GetInfo (inexisting element, wildcard allowed, wildcard exists).");

    foundInfo = service->GetInfo(prop4, "unknownmime", false);
    MITK_TEST_CONDITION_REQUIRED(foundInfo.IsNull(), "Check GetInfo (inexisting element, no wildcard allowed).");
    foundInfo = service->GetInfo(prop4, "unknownmime", true);
    MITK_TEST_CONDITION_REQUIRED(foundInfo.IsNull(), "Check GetInfo (inexisting element, wildcard allowed).");
  }

  void HasInfos()
  {
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop1), "Check HasInfos (prop1)");
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop4), "Check HasInfos (prop4)");
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos("unkownProp"), "Check HasInfos (unkown prop)");
  }

  void RemoveAllInfos()
  {
    service->RemoveAllInfos();
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop1), "Check HasInfos (prop1)");
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop4), "Check HasInfos (prop4)");
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop5), "Check HasInfos (prop5)");
  }


  void RemoveInfos()
  {
    service->RemoveInfos(prop1);
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop1), "Check HasInfos (prop1)");
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop4), "Check HasInfos (prop4)");
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop5), "Check HasInfos (prop5)");

    service->RemoveInfos(prop4);
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop4), "Check HasInfos (prop4)");
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop5), "Check HasInfos (prop5)");

    service->RemoveInfos(prop5);
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop5), "Check HasInfos (prop5)");

    service->RemoveInfos("unknown_prop");
  }

  void RemoveInfos_withMime()
  {
    service->RemoveInfos(prop1, "mime2");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo(prop1,"mime2",false).IsNull(), "Check RemoveInfos if info was removed");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo(prop1, "mime3", false).IsNotNull(), "Check RemoveInfos, if other info of same property name still exists");
    MITK_TEST_CONDITION_REQUIRED(service->GetInfo(prop4, "mime2", false).IsNotNull(), "Check RemoveInfos, if other info of other property name but same mime still exists");

    service->RemoveInfos(prop5, "wrongMime");
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop5), "Check RemoveInfos on prop 5 with wrong mime");

    service->RemoveInfos(prop5, "mime5");
    MITK_TEST_CONDITION_REQUIRED(!service->HasInfos(prop5), "Check RemoveInfos on prop 5");

    service->RemoveInfos("unkown_prop", "mime2");
    MITK_TEST_CONDITION_REQUIRED(service->HasInfos(prop4), "Check RemoveInfos, if unkown property name but exting mime was used");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyPersistence)
