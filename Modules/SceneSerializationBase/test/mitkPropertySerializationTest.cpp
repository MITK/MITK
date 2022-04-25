/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"

#include "mitkCoreObjectFactory.h"

#include "mitkBaseProperty.h"
#include "mitkProperties.h"
#include <mitkAnnotationProperty.h>
#include <mitkClippingProperty.h>
#include <mitkColorProperty.h>
#include <mitkEnumerationProperty.h>
#include <mitkModalityProperty.h>
#include <mitkGroupTagProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPlaneOrientationProperty.h>
#include <mitkStringProperty.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkVtkScalarModeProperty.h>

#include "mitkBasePropertySerializer.h"
#include "mitkPropertyList.h"
#include "mitkPropertyListSerializer.h"

#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>
#include <mitkVtkWidgetRendering.h>

#include <tinyxml2.h>

void TestAllProperties(const mitk::PropertyList *propList);

/**Documentation
* \brief Test for all PropertySerializer classes.
*
*/
int mitkPropertySerializationTest(int /* argc */, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("PropertySerializationTest");

  mitk::PropertyListSerializer::Pointer serializer =
    mitk::PropertyListSerializer::New(); // make sure something from the lib is actually used (registration of
                                         // serializers)

  /* build list of properties that will be serialized and deserialized */
  mitk::PropertyList::Pointer propList = mitk::PropertyList::New();
  propList->SetProperty("booltrue", mitk::BoolProperty::New(true));
  propList->SetProperty("boolfalse", mitk::BoolProperty::New(false));
  propList->SetProperty("int", mitk::IntProperty::New(-32));
  propList->SetProperty("float", mitk::FloatProperty::New(-31.337));
  propList->SetProperty("double", mitk::DoubleProperty::New(-31.337));
  propList->SetProperty("string", mitk::StringProperty::New("Hello MITK"));
  mitk::Point3D p3d;
  mitk::FillVector3D(p3d, 1.0, 2.2, -3.3);
  propList->SetProperty("p3d", mitk::Point3dProperty::New(p3d));
  mitk::Point3I p3i;
  mitk::FillVector3D(p3i, 1, 2, -3);
  propList->SetProperty("p3i", mitk::Point3iProperty::New(p3i));
  mitk::Point4D p4d;
  mitk::FillVector4D(p4d, 1.5, 2.6, -3.7, 4.44);
  propList->SetProperty("p4d", mitk::Point4dProperty::New(p4d));
  mitk::Vector3D v3d;
  mitk::FillVector3D(v3d, 1.0, 2.2, -3.3);
  propList->SetProperty("v3d", mitk::Vector3DProperty::New(v3d));
  propList->SetProperty("annotation", mitk::AnnotationProperty::New("My Annotation", p3d));
  propList->SetProperty("clipping", mitk::ClippingProperty::New(p3d, v3d));
  propList->SetProperty("color", mitk::ColorProperty::New(1.0, 0.2, 0.2));
  propList->SetProperty("modality", mitk::ModalityProperty::New("Color Doppler"));
  propList->SetProperty("PlaneOrientationProperty",
                        mitk::PlaneOrientationProperty::New("Arrows in positive direction"));
  propList->SetProperty("VtkInterpolationProperty", mitk::VtkInterpolationProperty::New("Gouraud"));
  propList->SetProperty("VtkRepresentationProperty", mitk::VtkRepresentationProperty::New("Surface"));
  propList->SetProperty("VtkResliceInterpolationProperty", mitk::VtkResliceInterpolationProperty::New("Cubic"));
  propList->SetProperty("VtkScalarModeProperty", mitk::VtkScalarModeProperty::New("PointFieldData"));
  mitk::BoolLookupTable blt;
  blt.SetTableValue(0, true);
  blt.SetTableValue(1, false);
  blt.SetTableValue(2, true);
  propList->SetProperty("BoolLookupTableProperty", mitk::BoolLookupTableProperty::New(blt));
  mitk::FloatLookupTable flt;
  flt.SetTableValue(0, 3.1);
  flt.SetTableValue(1, 3.3);
  flt.SetTableValue(2, 7.0);
  propList->SetProperty("FloatLookupTableProperty", mitk::FloatLookupTableProperty::New(flt));
  mitk::IntLookupTable ilt;
  ilt.SetTableValue(0, 3);
  ilt.SetTableValue(1, 2);
  ilt.SetTableValue(2, 11);
  propList->SetProperty("IntLookupTableProperty", mitk::IntLookupTableProperty::New(ilt));
  mitk::StringLookupTable slt;
  slt.SetTableValue(0, "Hello");
  slt.SetTableValue(1, "MITK");
  slt.SetTableValue(2, "world");
  propList->SetProperty("StringLookupTableProperty", mitk::StringLookupTableProperty::New(slt));
  propList->SetProperty("GroupTagProperty", mitk::GroupTagProperty::New());
  propList->SetProperty("LevelWindowProperty", mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 50.0)));
  mitk::LookupTable::Pointer lt = mitk::LookupTable::New();
  lt->ChangeOpacityForAll(0.25);
  lt->ChangeOpacity(17, 0.88);
  propList->SetProperty("LookupTableProperty", mitk::LookupTableProperty::New(lt));
  propList->SetProperty("StringProperty", mitk::StringProperty::New("Oh why, gruel world"));

  MITK_TEST_CONDITION_REQUIRED(propList->GetMap()->size() > 0, "Initialize PropertyList");

  TestAllProperties(propList);

  /* test default property lists of basedata objects */
  // activate the following tests after MaterialProperty is deleted

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(mitk::PointSet::New());
  TestAllProperties(node->GetPropertyList());
  node->SetData(mitk::Image::New());
  TestAllProperties(node->GetPropertyList());
  node->SetData(mitk::Surface::New());
  TestAllProperties(node->GetPropertyList());
  node->SetData(mitk::VtkWidgetRendering::New());
  TestAllProperties(node->GetPropertyList());

  MITK_TEST_END();
}

void TestAllProperties(const mitk::PropertyList *propList)
{
  assert(propList);

  /* try to serialize each property in the list, then deserialize again and check for equality */
  for (auto it = propList->GetMap()->begin();
       it != propList->GetMap()->end();
       ++it)
  {
    const mitk::BaseProperty *prop = it->second;
    // construct name of serializer class
    std::string serializername = std::string(prop->GetNameOfClass()) + "Serializer";
    std::list<itk::LightObject::Pointer> allSerializers =
      itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
    MITK_TEST_CONDITION(allSerializers.size() > 0, std::string("Creating serializers for ") + serializername);
    if (allSerializers.size() == 0)
    {
      MITK_TEST_OUTPUT(<< "serialization not possible, skipping " << prop->GetNameOfClass());
      continue;
    }
    if (allSerializers.size() > 1)
    {
      MITK_TEST_OUTPUT(<< "Warning: " << allSerializers.size() << " serializers found for " << prop->GetNameOfClass()
                       << "testing only the first one.");
    }
    auto *serializer =
      dynamic_cast<mitk::BasePropertySerializer *>(allSerializers.begin()->GetPointer());
    MITK_TEST_CONDITION(serializer != nullptr, serializername + std::string(" is valid"));
    if (serializer != nullptr)
    {
      serializer->SetProperty(prop);
      tinyxml2::XMLDocument doc;
      tinyxml2::XMLElement *valueelement = nullptr;
      try
      {
        valueelement = serializer->Serialize(doc);
      }
      catch (...)
      {
      }
      MITK_TEST_CONDITION(valueelement != nullptr, std::string("Serialize property with ") + serializername);

      if (valueelement == nullptr)
      {
        MITK_TEST_OUTPUT(<< "serialization failed, skipping deserialization");
        continue;
      }

      mitk::BaseProperty::Pointer deserializedProp = serializer->Deserialize(valueelement);
      MITK_TEST_CONDITION(deserializedProp.IsNotNull(), "serializer created valid property");
      if (deserializedProp.IsNotNull())
      {
        MITK_TEST_CONDITION(*(deserializedProp.GetPointer()) == *prop,
                            "deserialized property equals initial property for type " << prop->GetNameOfClass());
      }
    }
    else
    {
      MITK_TEST_OUTPUT(<< "created serializer object is of class "
                       << allSerializers.begin()->GetPointer()->GetNameOfClass())
    }
  } // for all properties
}
