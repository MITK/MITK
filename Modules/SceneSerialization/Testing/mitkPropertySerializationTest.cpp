/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkTestingMacros.h"

#include "mitkDataTreeNodeFactory.h"
#include "mitkCoreObjectFactory.h"

#include "mitkBaseProperty.h"
#include "mitkProperties.h"
#include <mitkAnnotationProperty.h>
#include <mitkClippingProperty.h>
#include <mitkColorProperty.h>
#include <mitkEnumerationProperty.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkModalityProperty.h>
//#include <mitkOdfNormalizationMethodProperty.h>
//#include <mitkOdfScaleByProperty.h>
#include <mitkOrganTypeProperty.h>
#include <mitkPlaneDecorationProperty.h>
#include <mitkShaderEnumProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkVtkScalarModeProperty.h>
#include <mitkVtkVolumeRenderingProperty.h>
#include <mitkGroupTagProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkStringProperty.h>
#include <mitkDelegateProperty.h>
#include <mitkTransferFunctionProperty.h>

#include "mitkPropertyList.h"
#include "mitkBasePropertySerializer.h"
#include "mitkBasePropertyDeserializer.h"

#include <mitkContour.h>
#include <mitkContourSet.h>
#include <mitkPointData.h>
#include <mitkPointSet.h>
#include <mitkMesh.h>
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkCone.h>
#include <mitkCuboid.h>
#include <mitkCylinder.h>
#include <mitkEllipsoid.h>
#include <mitkExtrudedContour.h>
#include <mitkPlane.h>
//#include <mitkTrackingVolume.h>
#include <mitkUnstructuredGrid.h>
#include <mitkVtkWidgetRendering.h>


void TestAllProperties(const mitk::PropertyList* propList);

/**Documentation
* \brief Test for all PropertySerializer and PropertyDeserializer classes. 
*
*/
int mitkPropertySerializationTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PropertySerializationTest");

  itk::ObjectFactoryBase::RegisterFactory(mitk::CoreObjectFactory::New());


  /* build list of properties that will be serialized and deserialized */
  mitk::PropertyList::Pointer propList = mitk::PropertyList::New();
  propList->SetProperty("bool", mitk::BoolProperty::New(true));
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
  //mitk::EnumerationProperty::Pointer en = mitk::EnumerationProperty::New();
  //en->AddEnum("PC", 1); en->AddEnum("Playstation", 2); en->AddEnum("Wii", 111); en->AddEnum("XBox", 7);
  //en->SetValue("XBox");
  //propList->SetProperty("enum", en);
  propList->SetProperty("gridrep", mitk::GridRepresentationProperty::New(2));
  propList->SetProperty("gridvol", mitk::GridVolumeMapperProperty::New(0));
  propList->SetProperty("modality", mitk::ModalityProperty::New("Color Doppler"));
  //propList->SetProperty("OdfNormalizationMethodProperty", mitk::OdfNormalizationMethodProperty::New("Global Maximum"));
  //propList->SetProperty("OdfScaleByProperty", mitk::OdfScaleByProperty::New("Principal Curvature"));
  propList->SetProperty("OrganTypeProperty", mitk::OrganTypeProperty::New("Larynx"));
  propList->SetProperty("PlaneDecorationProperty", mitk::PlaneDecorationProperty::New("Arrows in positive direction"));
  propList->SetProperty("ShaderEnumProperty", mitk::ShaderEnumProperty::New("fixed"));
  propList->SetProperty("VtkInterpolationProperty", mitk::VtkInterpolationProperty::New("Gouraud"));
  propList->SetProperty("VtkRepresentationProperty", mitk::VtkRepresentationProperty::New("Surface"));
  propList->SetProperty("VtkResliceInterpolationProperty", mitk::VtkResliceInterpolationProperty::New("Cubic"));
  propList->SetProperty("VtkScalarModeProperty", mitk::VtkScalarModeProperty::New("PointFieldData"));
  propList->SetProperty("VtkVolumeRenderingProperty", mitk::VtkVolumeRenderingProperty::New("COMPOSITE"));
  mitk::BoolLookupTable blt;
  blt.SetTableValue(0, true);  blt.SetTableValue(1, false);  blt.SetTableValue(2, true);
  propList->SetProperty("BoolLookupTableProperty", mitk::BoolLookupTableProperty::New(blt));
  mitk::FloatLookupTable flt;
  flt.SetTableValue(0, 3.1);  flt.SetTableValue(1, 3.3);  flt.SetTableValue(2, 7.0);
  propList->SetProperty("FloatLookupTableProperty", mitk::FloatLookupTableProperty::New(flt));
  mitk::IntLookupTable ilt;
  ilt.SetTableValue(0, 3);  ilt.SetTableValue(1, 2);  ilt.SetTableValue(2, 11);
  propList->SetProperty("IntLookupTableProperty", mitk::IntLookupTableProperty::New(ilt));
  mitk::StringLookupTable slt;
  slt.SetTableValue(0, "Hello");  slt.SetTableValue(1, "MITK");  slt.SetTableValue(2, "world");
  propList->SetProperty("StringLookupTableProperty", mitk::StringLookupTableProperty::New(slt));
  propList->SetProperty("GroupTagProperty", mitk::GroupTagProperty::New());
  propList->SetProperty("LevelWindowProperty", mitk::LevelWindowProperty::New(mitk::LevelWindow(100.0, 50.0)));
  mitk::LookupTable::Pointer lt = mitk::LookupTable::New();
  lt->ChangeOpacityForAll(0.25); 
  lt->ChangeOpacity(17, 0.88);
  propList->SetProperty("LookupTableProperty", mitk::LookupTableProperty::New(lt));
  propList->SetProperty("StringProperty", mitk::StringProperty::New("Oh why, gruel world"));
  propList->SetProperty("DelegateProperty", mitk::DelegateProperty::New("Do it!"));
  mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
  tf->SetTransferFunctionMode(1);
  propList->SetProperty("TransferFunctionProperty", mitk::TransferFunctionProperty::New(tf));


  MITK_TEST_CONDITION_REQUIRED(propList->GetMap()->size() > 0, "Initialize PropertyList");

  TestAllProperties(propList);
  
  /* test default property lists of basedata objects */
  // activate the following tests after MaterialProperty is deleted
  //mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
  //node->SetData(mitk::Contour::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::ContourSet::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::PointData::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::PointSet::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Mesh::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Image::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Surface::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Cone::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Cuboid::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Cylinder::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Ellipsoid::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::ExtrudedContour::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::Plane::New());
  //TestAllProperties(node->GetPropertyList());
  ////node->SetData(mitk::TrackingVolume::New());  // TrackingVolume is in IGT Module, it does not have special properties, therefore we skip it here
  ////TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::UnstructuredGrid::New());
  //TestAllProperties(node->GetPropertyList());
  //node->SetData(mitk::VtkWidgetRendering::New());
  //TestAllProperties(node->GetPropertyList());

/* not tested base data types: 
  BaseDataImplementation
  ColoredRectangleRendering
  mitk::DiffusionVolumes< TPixelType >
  GeometryData
  mitk::Geometry2DData
  GradientBackground
  ItkBaseDataAdapter
  LogoRendering
  SlicedData
  QBallImage
  SeedsImage
  TensorImage 
  BoundingObject
  BoundingObjectGroup 
  */

  MITK_TEST_END();
}

void TestAllProperties(const mitk::PropertyList* propList)
{
  assert(propList);

  /* try to serialize each property in the list, then deserialize again and check for equality */
  for (mitk::PropertyList::PropertyMap::const_iterator it = propList->GetMap()->begin(); it != propList->GetMap()->end(); ++it)
  {
    const mitk::BaseProperty* prop = it->second.first;
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
    MITK_TEST_CONDITION(serializer != NULL, serializername + std::string(" is valid"));
    if (serializer != NULL)
    {
      serializer->SetProperty(prop);
      TiXmlElement* valueelement = NULL;
      try
      {
        valueelement = serializer->Serialize();
      }
      catch (...)
      {
      }
      MITK_TEST_CONDITION(valueelement != NULL, std::string("Serialize property with ") + serializername);

      if (valueelement == NULL)
      {
        MITK_TEST_OUTPUT( << "serialization failed, skipping deserialization");
        continue;
      }
      /* build deserializer and try to deserialize property */
      std::string deserializerName = std::string(prop->GetNameOfClass()) + std::string("Deserializer");
      std::list<itk::LightObject::Pointer> allDeserializers = itk::ObjectFactoryBase::CreateAllInstance(deserializerName.c_str());
      MITK_TEST_CONDITION(allDeserializers.size() > 0, std::string("Creating deserializers for ") + deserializerName);
      if (allDeserializers.size() == 0)
      {
        MITK_TEST_OUTPUT( << "deserialization not possible, skipping deserialization of " << prop->GetNameOfClass());
        continue;
      }
      if (allDeserializers.size() > 1)
      {
        MITK_TEST_OUTPUT (<< "Warning: " << allDeserializers.size() << " deserializers found for " << prop->GetNameOfClass() << "testing only the first one.");
      }
      mitk::BasePropertyDeserializer* deserializer = dynamic_cast<mitk::BasePropertyDeserializer*>( allDeserializers.begin()->GetPointer());
      MITK_TEST_CONDITION(deserializer != NULL, deserializerName + std::string(" is valid"));
      if (deserializer != NULL)
      {
        mitk::BaseProperty::Pointer deserializedProp = deserializer->Deserialize( valueelement );
        MITK_TEST_CONDITION(deserializedProp.IsNotNull(), "deserializer created valid property");
        if (deserializedProp.IsNotNull())
        {
          MITK_TEST_CONDITION(*(deserializedProp.GetPointer()) == *prop, "deserialized property equals initial property for type " << prop->GetNameOfClass());
        }
      }
    }
    else
    {
      MITK_TEST_OUTPUT( << "created serializer object is of class " << allSerializers.begin()->GetPointer()->GetNameOfClass())
    }
  } // for all properties
}
