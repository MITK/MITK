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

#include "mitkIOUtil.h"
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include <mitkBasePropertySerializer.h>

#include <mitkBValueMapProperty.h>

#include <mitkGradientDirectionsProperty.h>
#include <mitkMeasurementFrameProperty.h>

#include <mitkDiffusionPropertyHelper.h>

class mitkDiffusionPropertySerializerTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkDiffusionPropertySerializerTestSuite);
  MITK_TEST(Equal_SerializeandDeserialize_ReturnsTrue);

  //MITK_TEST(Equal_DifferentChannels_ReturnFalse);


  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::PropertyList::Pointer propList; //represet image propertylist
  mitk::BValueMapProperty::Pointer bvaluemap_prop;
  mitk::GradientDirectionsProperty::Pointer gradientdirection_prop;
  mitk::MeasurementFrameProperty::Pointer measurementframe_prop;

public:

  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp() override
  {

    propList = mitk::PropertyList::New();

    mitk::BValueMapProperty::BValueMap map;
    std::vector<unsigned int> indices1;
    indices1.push_back(1);
    indices1.push_back(2);
    indices1.push_back(3);
    indices1.push_back(4);

    map[0] = indices1;
    std::vector<unsigned int> indices2;
    indices2.push_back(4);
    indices2.push_back(3);
    indices2.push_back(2);
    indices2.push_back(1);

    map[1000] = indices2;
    bvaluemap_prop = mitk::BValueMapProperty::New(map).GetPointer();
    propList->SetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str(),bvaluemap_prop);


    mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer gdc;
    gdc = mitk::GradientDirectionsProperty::GradientDirectionsContainerType::New();

    double a[3] = {3.0,4.0,1.4};
    vnl_vector_fixed<double,3> vec1;
    vec1.set(a);
    gdc->push_back(vec1);

    double b[3] = {1.0,5.0,123.4};
    vnl_vector_fixed<double,3> vec2;
    vec2.set(b);
    gdc->push_back(vec2);

    double c[3] = {13.0,84.02,13.4};
    vnl_vector_fixed<double,3> vec3;
    vec3.set(c);
    gdc->push_back(vec3);


    gradientdirection_prop = mitk::GradientDirectionsProperty::New(gdc.GetPointer()).GetPointer();
    propList->ReplaceProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), gradientdirection_prop);

    mitk::MeasurementFrameProperty::MeasurementFrameType mft;

    double row0[3] = {1,0,0};
    double row1[3] = {0,1,0};
    double row2[3] = {0,0,1};

    mft.set_row(0,row0);
    mft.set_row(1,row1);
    mft.set_row(2,row2);

    measurementframe_prop = mitk::MeasurementFrameProperty::New(mft).GetPointer();
    propList->ReplaceProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), measurementframe_prop);
  }

  void tearDown() override
  {

  }

  void Equal_SerializeandDeserialize_ReturnsTrue()
  {

    assert(propList);

    /* try to serialize each property in the list, then deserialize again and check for equality */
    for (mitk::PropertyList::PropertyMap::const_iterator it = propList->GetMap()->begin(); it != propList->GetMap()->end(); ++it)
    {
      const mitk::BaseProperty* prop = it->second;
      // construct name of serializer class
      std::string serializername = std::string(prop->GetNameOfClass()) + "Serializer";
      std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
      MITK_TEST_CONDITION(allSerializers.size() > 0, std::string("Creating serializers for ") + serializername);
      if (allSerializers.size() == 0)
      {
        MITK_TEST_OUTPUT( << "serialization not possible, skipping " << prop->GetNameOfClass());
        continue;
      }
      if (allSerializers.size() > 1)
      {
        MITK_TEST_OUTPUT (<< "Warning: " << allSerializers.size() << " serializers found for " << prop->GetNameOfClass() << "testing only the first one.");
      }
      mitk::BasePropertySerializer* serializer = dynamic_cast<mitk::BasePropertySerializer*>( allSerializers.begin()->GetPointer());
      MITK_TEST_CONDITION(serializer != nullptr, serializername + std::string(" is valid"));
      if (serializer != nullptr)
      {
        serializer->SetProperty(prop);
        TiXmlElement* valueelement = nullptr;
        try
        {
          valueelement = serializer->Serialize();
//          TiXmlPrinter p;
//          valueelement->Accept(&p);
//          MITK_INFO << p.CStr();
        }
        catch (...)
        {
        }
        MITK_TEST_CONDITION(valueelement != nullptr, std::string("Serialize property with ") + serializername);

        if (valueelement == nullptr)
        {
          MITK_TEST_OUTPUT( << "serialization failed, skipping deserialization");
          continue;
        }

        mitk::BaseProperty::Pointer deserializedProp = serializer->Deserialize( valueelement );
        MITK_TEST_CONDITION(deserializedProp.IsNotNull(), "serializer created valid property");
        if (deserializedProp.IsNotNull())
        {
          MITK_TEST_CONDITION(*(deserializedProp.GetPointer()) == *prop, "deserialized property equals initial property for type " << prop->GetNameOfClass());
        }

      }
      else
      {
        MITK_TEST_OUTPUT( << "created serializer object is of class " << allSerializers.begin()->GetPointer()->GetNameOfClass())
      }
    } // for all properties

  }


};

MITK_TEST_SUITE_REGISTRATION(mitkDiffusionPropertySerializer)
