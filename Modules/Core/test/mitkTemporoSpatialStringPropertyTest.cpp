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

  MITK_TEST(serializeTemporoSpatialStringPropertyToJSON);
  MITK_TEST(deserializeJSONToTemporoSpatialStringProperty);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string refJSON;
  mitk::TemporoSpatialStringProperty::Pointer refProp;

public:
  void setUp() override
  {
    refJSON = "{\"values\":[{\"t\":0, \"z\":0, \"value\":\"v_0_0\"}, {\"t\":3, \"z\":0, \"value\":\"v_3_0\"}, "
              "{\"t\":3, \"z\":2, \"value\":\"v_3_2\"}, {\"t\":6, \"z\":1, \"value\":\"v_6_1\"}]}";
    refProp = mitk::TemporoSpatialStringProperty::New();
    refProp->SetValue(0, 0, "v_0_0");
    refProp->SetValue(3, 0, "v_3_0");
    refProp->SetValue(3, 2, "v_3_2");
    refProp->SetValue(6, 1, "v_6_1");
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

  void serializeTemporoSpatialStringPropertyToJSON()
  {
    std::string data = mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(refProp);
    CPPUNIT_ASSERT(refJSON ==
                   data); //"Testing serializeTemporoSpatialStringPropertyToJSON() producing correct string.");
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
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkTemporoSpatialStringProperty)
