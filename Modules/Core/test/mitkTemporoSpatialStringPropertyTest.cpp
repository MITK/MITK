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

  MITK_TEST(serializeTemporoSpatialStringPropertyToJSON);
  MITK_TEST(deserializeJSONToTemporoSpatialStringProperty);

  CPPUNIT_TEST_SUITE_END();

private:

  std::string refJSON;
  mitk::TemporoSpatialStringProperty::Pointer refProp;

public:

  void setUp() override
  {
    refJSON = "{\"values\":[{\"t\":0, \"z\":0, \"value\":\"v_0_0\"}, {\"t\":3, \"z\":0, \"value\":\"v_3_0\"}, {\"t\":3, \"z\":2, \"value\":\"v_3_2\"}, {\"t\":6, \"z\":1, \"value\":\"v_6_1\"}]}";
    refProp = mitk::TemporoSpatialStringProperty::New();
    refProp->SetValue(0, 0, "v_0_0");
    refProp->SetValue(3, 0, "v_3_0");
    refProp->SetValue(3, 2, "v_3_2");
    refProp->SetValue(6, 1, "v_6_1");
  }

  void tearDown() override
  {
  }


  void serializeTemporoSpatialStringPropertyToJSON()
  {
    std::string data = mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(refProp);
    MITK_TEST_CONDITION_REQUIRED(refJSON == data, "Testing serializeTemporoSpatialStringPropertyToJSON() producing correct string.");
  }

  void deserializeJSONToTemporoSpatialStringProperty()
  {
    mitk::BaseProperty::Pointer prop = mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(refJSON);

    mitk::TemporoSpatialStringProperty* tsProp = dynamic_cast<mitk::TemporoSpatialStringProperty*>(prop.GetPointer());

    MITK_TEST_CONDITION_REQUIRED(tsProp->GetValue(0,0) == "v_0_0", "Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 1");
    MITK_TEST_CONDITION_REQUIRED(tsProp->GetValue(3, 0) == "v_3_0", "Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 2");
    MITK_TEST_CONDITION_REQUIRED(tsProp->GetValue(3, 2) == "v_3_2", "Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 3");
    MITK_TEST_CONDITION_REQUIRED(tsProp->GetValue(6, 1) == "v_6_1", "Testing deserializeJSONToTemporoSpatialStringProperty() producing property with correct value 4");
    MITK_TEST_CONDITION_REQUIRED(*tsProp == *refProp, "Testing deserializeJSONToTemporoSpatialStringProperty()");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTemporoSpatialStringProperty)
