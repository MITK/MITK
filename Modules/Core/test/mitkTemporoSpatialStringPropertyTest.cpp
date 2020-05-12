/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTemporoSpatialStringProperty.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>

class mitkTemporoSpatialStringPropertyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTemporoSpatialStringPropertyTestSuite);

  MITK_TEST(GetValue);
  MITK_TEST(HasValue);
  MITK_TEST(SetValue);
  MITK_TEST(IsUniform);

  MITK_TEST(serializeTemporoSpatialStringPropertyToJSON);
  MITK_TEST(deserializeJSONToTemporoSpatialStringProperty);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string refJSON;
  std::string refJSON_legacy;
  std::string refPartlyCondensibleJSON;
  std::string refCondensibleJSON;
  mitk::TemporoSpatialStringProperty::Pointer refProp;
  mitk::TemporoSpatialStringProperty::Pointer refPartlyCondensibleProp;
  mitk::TemporoSpatialStringProperty::Pointer refCondensibleProp;

public:
  void setUp() override
  {
    refJSON_legacy = "{\"values\":[{\"t\":0, \"z\":0, \"value\":\"v_0_0\"}, {\"t\":3, \"z\":0, \"value\":\"v_3_0\"}, "
                     "{\"t\":3, \"z\":2, \"value\":\"v_3_2\"}, {\"t\":6, \"z\":1, \"value\":\"v_6_1\"}]}";
    refJSON = "{\"values\":[{\"z\":0, \"t\":0, \"value\":\"v_0_0\"}, {\"z\":0, \"t\":3, \"value\":\"v_3_0\"}, {\"z\":1, \"t\":6, \"value\":\"v_6_1\"}, {\"z\":2, \"t\":3, \"value\":\"v_3_2\"}]}";
    refPartlyCondensibleJSON = "{\"values\":[{\"z\":0, \"t\":0, \"value\":\"0\"}, {\"z\":1, \"t\":0, \"tmax\":1, \"value\":\"0\"}, {\"z\":1, \"t\":3, \"tmax\":5, \"value\":\"0\"}, {\"z\":1, \"t\":6, \"value\":\"otherValue\"}, {\"z\":2, \"t\":6, \"value\":\"0\"}]}";
    refCondensibleJSON = "{\"values\":[{\"z\":0, \"zmax\":2, \"t\":0, \"tmax\":1, \"value\":\"1\"}]}";
    refProp = mitk::TemporoSpatialStringProperty::New();
    refProp->SetValue(0, 0, "v_0_0");
    refProp->SetValue(3, 0, "v_3_0");
    refProp->SetValue(3, 2, "v_3_2");
    refProp->SetValue(6, 1, "v_6_1");

    refPartlyCondensibleProp = mitk::TemporoSpatialStringProperty::New();
    refPartlyCondensibleProp->SetValue(0, 0, "0");
    refPartlyCondensibleProp->SetValue(0, 1, "0");
    refPartlyCondensibleProp->SetValue(1, 1, "0");
    refPartlyCondensibleProp->SetValue(3, 1, "0");
    refPartlyCondensibleProp->SetValue(4, 1, "0");
    refPartlyCondensibleProp->SetValue(5, 1, "0");
    refPartlyCondensibleProp->SetValue(6, 1, "otherValue");
    refPartlyCondensibleProp->SetValue(6, 2, "0");

    refCondensibleProp = mitk::TemporoSpatialStringProperty::New();
    refCondensibleProp->SetValue(0, 0, "1");
    refCondensibleProp->SetValue(1, 0, "1");
    refCondensibleProp->SetValue(0, 1, "1");
    refCondensibleProp->SetValue(1, 1, "1");
    refCondensibleProp->SetValue(0, 2, "1");
    refCondensibleProp->SetValue(1, 2, "1");
  }

  void tearDown() override {}

  void GetValue()
  {
    CPPUNIT_ASSERT(refProp->GetValue() == "v_0_0");

    CPPUNIT_ASSERT(refProp->GetValue(3, 0) == "v_3_0");
    CPPUNIT_ASSERT(refProp->GetValue(3, 2) == "v_3_2");
    CPPUNIT_ASSERT(refProp->GetValue(3, 1, false, true) == "v_3_0");
    CPPUNIT_ASSERT(refProp->GetValue(3, 5, false, true) == "v_3_2");

    CPPUNIT_ASSERT(refProp->GetValueBySlice(0) == "v_0_0");
    CPPUNIT_ASSERT(refProp->GetValueBySlice(4, true) == "v_0_0");

    CPPUNIT_ASSERT(refProp->GetValueByTimeStep(3) == "v_3_0");
    CPPUNIT_ASSERT(refProp->GetValueByTimeStep(6) == "v_6_1");
    CPPUNIT_ASSERT(refProp->GetValueByTimeStep(5, true) == "v_3_0");

    CPPUNIT_ASSERT(refProp->GetValueAsString() == "v_0_0");

    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps().size() == 3);
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps()[0] == 0);
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps()[1] == 3);
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps()[2] == 6);

    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps(0).size() == 2);
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps(0)[0] == 0);
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps(0)[1] == 3);

    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps(1).size() == 1);
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps(1)[0] == 6);

    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps(5).size() == 0);

    CPPUNIT_ASSERT(refProp->GetAvailableSlices().size() == 3);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices()[0] == 0);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices()[1] == 1);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices()[2] == 2);

    CPPUNIT_ASSERT(refProp->GetAvailableSlices(0).size() == 1);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices(0)[0] == 0);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices(3).size() == 2);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices(3)[0] == 0);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices(3)[1] == 2);

    CPPUNIT_ASSERT(refProp->GetAvailableSlices(2).size() == 0);
  }

  void HasValue()
  {
    CPPUNIT_ASSERT(refProp->HasValue());

    CPPUNIT_ASSERT(refProp->HasValue(3, 0));
    CPPUNIT_ASSERT(refProp->HasValue(3, 2));
    CPPUNIT_ASSERT(refProp->HasValue(3, 1, false, true));
    CPPUNIT_ASSERT(refProp->HasValue(3, 5, false, true));
    CPPUNIT_ASSERT(!refProp->HasValue(3, 1));
    CPPUNIT_ASSERT(!refProp->HasValue(3, 5));
    CPPUNIT_ASSERT(refProp->HasValue(4, 2, true, true));
    CPPUNIT_ASSERT(refProp->HasValue(4, 2, true, false));
    CPPUNIT_ASSERT(!refProp->HasValue(4, 2, false, true));

    CPPUNIT_ASSERT(refProp->HasValueBySlice(0));
    CPPUNIT_ASSERT(refProp->HasValueBySlice(4, true));
    CPPUNIT_ASSERT(!refProp->HasValueBySlice(4));

    CPPUNIT_ASSERT(refProp->HasValueByTimeStep(3));
    CPPUNIT_ASSERT(refProp->HasValueByTimeStep(6));
    CPPUNIT_ASSERT(refProp->HasValueByTimeStep(5, true));
    CPPUNIT_ASSERT(!refProp->HasValueByTimeStep(5));
  }

  void SetValue()
  {
    CPPUNIT_ASSERT_NO_THROW(refProp->SetValue(8, 9, "v_8_9"));
    CPPUNIT_ASSERT(refProp->GetValue(8, 9) == "v_8_9");

    CPPUNIT_ASSERT_NO_THROW(refProp->SetValue("newValue"));
    CPPUNIT_ASSERT(refProp->GetValue(0, 0) == "newValue");
    CPPUNIT_ASSERT(refProp->GetAvailableTimeSteps().size() == 1);
    CPPUNIT_ASSERT(refProp->GetAvailableSlices(0).size() == 1);
  }

  void IsUniform()
  {
    CPPUNIT_ASSERT(!refProp->IsUniform());
    CPPUNIT_ASSERT(!refPartlyCondensibleProp->IsUniform());
    CPPUNIT_ASSERT(refCondensibleProp->IsUniform());
  }

  void serializeTemporoSpatialStringPropertyToJSON()
  {
    std::string data = mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(refProp);
    CPPUNIT_ASSERT(refJSON ==
      data); //"Testing serializeTemporoSpatialStringPropertyToJSON() producing correct string.");

    data = mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(refPartlyCondensibleProp);
    CPPUNIT_ASSERT(refPartlyCondensibleJSON ==
      data);

    data = mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(refCondensibleProp);
    CPPUNIT_ASSERT(refCondensibleJSON ==
      data);
  }

  void deserializeJSONToTemporoSpatialStringProperty()
  {
    mitk::BaseProperty::Pointer prop =
      mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(refJSON);

    auto *tsProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(prop.GetPointer());

    CPPUNIT_ASSERT(
      tsProp->GetValue(0, 0) ==
      "v_0_0"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 1");
    CPPUNIT_ASSERT(
      tsProp->GetValue(3, 0) ==
      "v_3_0"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 2");
    CPPUNIT_ASSERT(
      tsProp->GetValue(3, 2) ==
      "v_3_2"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 3");
    CPPUNIT_ASSERT(
      tsProp->GetValue(6, 1) ==
      "v_6_1"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 4");
    CPPUNIT_ASSERT(*tsProp == *refProp); //"Testing deserializeJSONToTemporoSpatialStringProperty()");

    prop = mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(refJSON_legacy);
    tsProp = dynamic_cast<mitk::TemporoSpatialStringProperty*>(prop.GetPointer());
    CPPUNIT_ASSERT(
      tsProp->GetValue(0, 0) ==
      "v_0_0"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 1");
    CPPUNIT_ASSERT(
      tsProp->GetValue(3, 0) ==
      "v_3_0"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 2");
    CPPUNIT_ASSERT(
      tsProp->GetValue(3, 2) ==
      "v_3_2"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 3");
    CPPUNIT_ASSERT(
      tsProp->GetValue(6, 1) ==
      "v_6_1"); //"Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 4");
    CPPUNIT_ASSERT(*tsProp == *refProp); //"Testing deserializeJSONToTemporoSpatialStringProperty()");


    prop = mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(refPartlyCondensibleJSON);
    tsProp = dynamic_cast<mitk::TemporoSpatialStringProperty*>(prop.GetPointer());
    CPPUNIT_ASSERT(tsProp->GetValue(0, 0) =="0");
    CPPUNIT_ASSERT(tsProp->GetValue(0, 1) == "0");
    CPPUNIT_ASSERT(tsProp->GetValue(1, 1) == "0");
    CPPUNIT_ASSERT(tsProp->GetValue(3, 1) == "0");
    CPPUNIT_ASSERT(tsProp->GetValue(4, 1) == "0");
    CPPUNIT_ASSERT(tsProp->GetValue(5, 1) == "0");
    CPPUNIT_ASSERT(tsProp->GetValue(6, 1) == "otherValue");
    CPPUNIT_ASSERT(tsProp->GetValue(6, 2) == "0");
    CPPUNIT_ASSERT(*tsProp == *refPartlyCondensibleProp);
    
    prop = mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(refCondensibleJSON);
    tsProp = dynamic_cast<mitk::TemporoSpatialStringProperty*>(prop.GetPointer());
    CPPUNIT_ASSERT(tsProp->GetValue(0, 0) == "1");
    CPPUNIT_ASSERT(tsProp->GetValue(1, 0) == "1");
    CPPUNIT_ASSERT(tsProp->GetValue(0, 1) == "1");
    CPPUNIT_ASSERT(tsProp->GetValue(1, 1) == "1");
    CPPUNIT_ASSERT(tsProp->GetValue(0, 2) == "1");
    CPPUNIT_ASSERT(tsProp->GetValue(1, 2) == "1");
    CPPUNIT_ASSERT(*tsProp == *refCondensibleProp);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkTemporoSpatialStringProperty)
