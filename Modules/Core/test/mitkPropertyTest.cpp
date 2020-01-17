/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
// MITK includes
#include <mitkAnnotationProperty.h>
#include <mitkClippingProperty.h>
#include <mitkColorProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkLookupTables.h>
#include <mitkProperties.h>
#include <mitkSmartPointerProperty.h>
#include <mitkStringProperty.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkWeakPointerProperty.h>
#include <mitkLogMacros.h>
// ITK includes
#include <itkCommand.h>
// VTK includes
#include <vtkDebugLeaks.h>

struct PropertyModifiedListener
{
  typedef itk::SimpleMemberCommand<PropertyModifiedListener> CmdType;

  PropertyModifiedListener() : m_Modified(false), m_Cmd(CmdType::New())
  {
    m_Cmd->SetCallbackFunction(this, &PropertyModifiedListener::Modified);
  }

  void Modified() { m_Modified = true; }
  bool Pop()
  {
    bool b = m_Modified;
    m_Modified = false;
    return b;
  }

  bool m_Modified;
  CmdType::Pointer m_Cmd;
};

class mitkPropertyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyTestSuite);
  MITK_TEST(TestBoolProperty_Success);
  MITK_TEST(TestIntProperty_Success);
  MITK_TEST(TestFloatProperty_Success);
  MITK_TEST(TestDoubleProperty_Success);
  MITK_TEST(TestVector3DProperty_Success);
  MITK_TEST(TestPoint3D_Success);
  MITK_TEST(TestPoint4D_Success);
  MITK_TEST(TestPoint3I_Success);
  MITK_TEST(TestFloatLookupTable_Success);
  MITK_TEST(TestBoolLookupTable_Success);
  MITK_TEST(TestIntLookupTable_Success);
  MITK_TEST(TestStringLookupTable_Success);
  MITK_TEST(TestAnnotationProperty_Success);
  MITK_TEST(TestClippingProperty_Success);
  MITK_TEST(TestColorProperty_Success);
  MITK_TEST(TestLevelWindowProperty_Success);
  MITK_TEST(TestSmartPointerProperty_Success);
  MITK_TEST(TestStringProperty_Success);
  MITK_TEST(TestTransferFunctionProperty_Success);
  MITK_TEST(TestWeakPointerProperty_Success);
  MITK_TEST(TestLookupTablePropertyProperty_Success);
  CPPUNIT_TEST_SUITE_END();

private:
  PropertyModifiedListener m_L;
public:
  void setUp()
  {
  }
  void tearDown()
  {
  }

  template <class T>
  void TestPropInequality(T prop, T prop2)
  {
    mitk::BaseProperty::Pointer baseProp2(prop2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Test inequality 1", !(*prop == *prop2));
    CPPUNIT_ASSERT_MESSAGE("Test polymorphic inequality 1", !(*prop == *baseProp2));
    CPPUNIT_ASSERT_MESSAGE("Test polymorphic inequality 2", !(*baseProp2 == *prop));
  }

  template <class T>
  void TestPropAssignment(T prop, T prop2, const std::string &strProp)
  {
    unsigned long tag = prop->AddObserver(itk::ModifiedEvent(), m_L.m_Cmd.GetPointer());

    mitk::BaseProperty::Pointer baseProp2(prop2.GetPointer());
    *prop = *baseProp2;
    CPPUNIT_ASSERT_MESSAGE("Test modified event", m_L.Pop());
    std::string msg = std::string("Test assignment [") + prop->GetValueAsString() + " == " + strProp + "]";
    CPPUNIT_ASSERT_MESSAGE(msg, prop->GetValueAsString() == strProp);
    CPPUNIT_ASSERT_MESSAGE("Test equality", *prop == *prop2);
    CPPUNIT_ASSERT_MESSAGE("Test equality", *prop == *baseProp2);
    CPPUNIT_ASSERT_MESSAGE("Test polymorphic equality", *baseProp2 == *prop);

    prop->RemoveObserver(tag);
  }

  template <class T>
  void TestPropPolymorphicAssignment(T prop, T prop2, const std::string &strProp)
  {
    mitk::BaseProperty::Pointer baseProp(prop.GetPointer());

    unsigned long tag = baseProp->AddObserver(itk::ModifiedEvent(), m_L.m_Cmd.GetPointer());
  
    *baseProp = *prop2;
    CPPUNIT_ASSERT_MESSAGE("Test modified event", m_L.Pop());
    std::string msg =
      std::string("Test polymorphic assignment [") + baseProp->GetValueAsString() + " == " + strProp + "]";
    CPPUNIT_ASSERT_MESSAGE(msg, baseProp->GetValueAsString() == strProp);
    CPPUNIT_ASSERT_MESSAGE("Test equality", *prop == *prop2);
    CPPUNIT_ASSERT_MESSAGE("Test equality", *prop2 == *baseProp);
    CPPUNIT_ASSERT_MESSAGE("Test polymorphic equality", *baseProp == *prop2);
    baseProp->RemoveObserver(tag);
  }
  
  template <class T>
  void TestPropCloning(T prop)
  {
    T prop2 = prop->Clone();
    CPPUNIT_ASSERT_MESSAGE("Test clone pointer", prop.GetPointer() != prop2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Test equality of the clone", *prop == *prop2);
  }
  
  template <class T>
  void TestProperty(const typename T::ValueType &v1,
                    const typename T::ValueType &v2,
                    const std::string &strV1,
                    const std::string &strV2)
  {
    typename T::Pointer prop = T::New(v1);
    MITK_TEST_OUTPUT(<< "**** Test [" << prop->GetNameOfClass() << "] ****");
  
    CPPUNIT_ASSERT_MESSAGE("Test constructor", prop->GetValue() == v1);
    std::string msg = std::string("Test GetValueAsString() [") + prop->GetValueAsString() + " == " + strV1 + "]";
    CPPUNIT_ASSERT_MESSAGE(msg, prop->GetValueAsString() == strV1);
  
    TestPropCloning(prop);
  
    typename T::Pointer prop2 = T::New();
    prop2->AddObserver(itk::ModifiedEvent(), m_L.m_Cmd.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Test modified", !m_L.m_Modified);
    prop2->SetValue(v2);
    CPPUNIT_ASSERT_MESSAGE("Test modified", m_L.Pop());
    CPPUNIT_ASSERT_MESSAGE("Test SetValue()", prop2->GetValue() == v2);
    prop2->SetValue(v2);
    CPPUNIT_ASSERT_MESSAGE("Test for no modification", !m_L.Pop());
  
    TestPropInequality(prop, prop2);
    TestPropAssignment(prop, prop2, strV2);
  
    prop->SetValue(v1);
    TestPropPolymorphicAssignment(prop2, prop, strV1);
  }

  void TestBoolProperty_Success()
  {
    TestProperty<mitk::BoolProperty>(false, true, "0", "1");
  }

  void TestIntProperty_Success()
  {
    TestProperty<mitk::IntProperty>(3, 5, "3", "5");
  }

  void TestFloatProperty_Success()
  {
    TestProperty<mitk::FloatProperty>(0.3f, -23.5f, "0.3", "-23.5");
  }

  void TestDoubleProperty_Success()
  {
    TestProperty<mitk::DoubleProperty>(64.1f, 2.34f, "64.1", "2.34");
  }

  void TestVector3DProperty_Success()
  {
    mitk::Vector3D p1;
    p1[0] = 2.0;
    p1[1] = 3.0;
    p1[2] = 4.0;
    mitk::Vector3D p2;
    p2[0] = -1.0;
    p2[1] = 2.0;
    p2[2] = 3.0;
    TestProperty<mitk::Vector3DProperty>(p1, p2, "[2, 3, 4]", "[-1, 2, 3]");
  }

  void TestPoint3D_Success()
  {
    mitk::Point3D p1;
    p1[0] = 2.0;
    p1[1] = 3.0;
    p1[2] = 4.0;
    mitk::Point3D p2;
    p2[0] = -1.0;
    p2[1] = 2.0;
    p2[2] = 3.0;
    TestProperty<mitk::Point3dProperty>(p1, p2, "[2, 3, 4]", "[-1, 2, 3]");
  }

  void TestPoint4D_Success()
  {
    mitk::Point4D p1;
    p1[0] = 2.0;
    p1[1] = 3.0;
    p1[2] = 4.0;
    p1[3] = -2.0;
    mitk::Point4D p2;
    p2[0] = -1.0;
    p2[1] = 2.0;
    p2[2] = 3.0;
    p2[3] = 5.0;
    TestProperty<mitk::Point4dProperty>(p1, p2, "[2, 3, 4, -2]", "[-1, 2, 3, 5]");
  }

  void TestPoint3I_Success()
  {
    mitk::Point3I p1;
    p1[0] = 2;
    p1[1] = 3;
    p1[2] = 4;
    mitk::Point3I p2;
    p2[0] = 8;
    p2[1] = 7;
    p2[2] = 6;
    TestProperty<mitk::Point3iProperty>(p1, p2, "[2, 3, 4]", "[8, 7, 6]");
  }

  void TestFloatLookupTable_Success()
  {
    mitk::FloatLookupTable lut1;
    lut1.SetTableValue(1, 0.3f);
    lut1.SetTableValue(4, 323.7f);

    mitk::FloatLookupTable lut2;
    lut2.SetTableValue(6, -0.3f);
    lut2.SetTableValue(2, 25.7f);

    TestProperty<mitk::FloatLookupTableProperty>(lut1, lut2, "[1 -> 0.3, 4 -> 323.7]", "[2 -> 25.7, 6 -> -0.3]");
  }

  void TestBoolLookupTable_Success()
  {
    mitk::BoolLookupTable lut1;
    lut1.SetTableValue(3, false);
    lut1.SetTableValue(5, true);

    mitk::BoolLookupTable lut2;
    lut2.SetTableValue(1, false);
    lut2.SetTableValue(2, false);
    TestProperty<mitk::BoolLookupTableProperty>(lut1, lut2, "[3 -> 0, 5 -> 1]", "[1 -> 0, 2 -> 0]");
  }

  void TestIntLookupTable_Success()
  {
    mitk::IntLookupTable lut1;
    lut1.SetTableValue(5, -12);
    lut1.SetTableValue(7, 3);

    mitk::IntLookupTable lut2;
    lut2.SetTableValue(4, -6);
    lut2.SetTableValue(8, -45);

    TestProperty<mitk::IntLookupTableProperty>(lut1, lut2, "[5 -> -12, 7 -> 3]", "[4 -> -6, 8 -> -45]");
  }

  void TestStringLookupTable_Success()
  {
    mitk::StringLookupTable lut1;
    lut1.SetTableValue(0, "a");
    lut1.SetTableValue(2, "b");

    mitk::StringLookupTable lut2;
    lut2.SetTableValue(0, "a");
    lut2.SetTableValue(2, "c");

    TestProperty<mitk::StringLookupTableProperty>(lut1, lut2, "[0 -> a, 2 -> b]", "[0 -> a, 2 -> c]");
  }

  void TestAnnotationProperty_Success()
  {
    std::string label1("Label1");
    mitk::Point3D point1;
    point1[0] = 3;
    point1[1] = 5;
    point1[2] = -4;
    std::string str1 = "Label1[3, 5, -4]";

    std::string label2("Label2");
    mitk::Point3D point2;
    point2[0] = -2;
    point2[1] = 8;
    point2[2] = -4;
    std::string str2 = "Label2[-2, 8, -4]";

    mitk::AnnotationProperty::Pointer prop = mitk::AnnotationProperty::New(label1, point1);
    MITK_TEST_OUTPUT(<< "**** Test [" << prop->GetNameOfClass() << "] ****");

    CPPUNIT_ASSERT_MESSAGE("Test constructor", prop->GetLabel() == label1 && prop->GetPosition() == point1);
    std::string msg = std::string("Test GetValueAsString() [") + prop->GetValueAsString() + " == " + str1 + "]";
    CPPUNIT_ASSERT_MESSAGE(msg, prop->GetValueAsString() == str1);

    mitk::AnnotationProperty::Pointer prop2 = mitk::AnnotationProperty::New();
    prop2->AddObserver(itk::ModifiedEvent(), m_L.m_Cmd.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Test not modified", !m_L.m_Modified);
    prop2->SetLabel(label2);
    CPPUNIT_ASSERT_MESSAGE("Test modified", m_L.Pop());
    prop2->SetPosition(point2);
    CPPUNIT_ASSERT_MESSAGE("Test modified", m_L.Pop());
    CPPUNIT_ASSERT_MESSAGE("Test Setter", prop2->GetLabel() == label2 && prop2->GetPosition() == point2);

    prop2->SetLabel(label2);
    CPPUNIT_ASSERT_MESSAGE("Test for no modification", !m_L.Pop());
    prop2->SetPosition(point2);
    CPPUNIT_ASSERT_MESSAGE("Test for no modification", !m_L.Pop());

    TestPropInequality(prop, prop2);
    TestPropAssignment(prop, prop2, str2);

    prop->SetLabel(label1);
    prop->SetPosition(point1);
    TestPropPolymorphicAssignment(prop2, prop, str1);
  }

  void TestClippingProperty_Success()
  {
    bool enabled1 = true;
    mitk::Point3D point1;
    point1[0] = 3;
    point1[1] = 5;
    point1[2] = -4;
    mitk::Vector3D vec1;
    vec1[0] = 0;
    vec1[1] = 2;
    vec1[2] = -1;
    std::string str1 = "1[3, 5, -4][0, 2, -1]";

    bool enabled2 = false;
    mitk::Point3D point2;
    point2[0] = -2;
    point2[1] = 8;
    point2[2] = -4;
    mitk::Vector3D vec2;
    vec2[0] = 0;
    vec2[1] = 2;
    vec2[2] = 4;
    std::string str2 = "0[-2, 8, -4][0, 2, 4]";

    mitk::ClippingProperty::Pointer prop = mitk::ClippingProperty::New(point1, vec1);
    MITK_TEST_OUTPUT(<< "**** Test [" << prop->GetNameOfClass() << "] ****");

    CPPUNIT_ASSERT_MESSAGE("Test constructor",
      prop->GetClippingEnabled() == enabled1 && prop->GetOrigin() == point1 && prop->GetNormal() == vec1);
    std::string msg = std::string("Test GetValueAsString() [") + prop->GetValueAsString() + " == " + str1 + "]";
    CPPUNIT_ASSERT_MESSAGE(msg, prop->GetValueAsString() == str1);

    mitk::ClippingProperty::Pointer prop2 = mitk::ClippingProperty::New();
    prop2->AddObserver(itk::ModifiedEvent(), m_L.m_Cmd.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Test not modified", !m_L.m_Modified);
    prop2->SetClippingEnabled(enabled2);
    CPPUNIT_ASSERT_MESSAGE("Test not modified", !m_L.Pop());
    prop2->SetOrigin(point2);
    CPPUNIT_ASSERT_MESSAGE("Test modified", m_L.Pop());
    prop2->SetNormal(vec2);
    CPPUNIT_ASSERT_MESSAGE("Test modified", m_L.Pop());

    CPPUNIT_ASSERT_MESSAGE("Test Setter",
      prop2->GetClippingEnabled() == enabled2 && prop2->GetOrigin() == point2 && prop2->GetNormal() == vec2);

    prop2->SetClippingEnabled(enabled2);
    CPPUNIT_ASSERT_MESSAGE("Test for no modification", !m_L.Pop());
    prop2->SetOrigin(point2);
    CPPUNIT_ASSERT_MESSAGE("Test for no modification", !m_L.Pop());
    prop2->SetNormal(vec2);
    CPPUNIT_ASSERT_MESSAGE("Test for no modification", !m_L.Pop());

    TestPropInequality(prop, prop2);
    TestPropAssignment(prop, prop2, str2);

    prop->SetClippingEnabled(enabled1);
    prop->SetOrigin(point1);
    prop->SetNormal(vec1);
    TestPropPolymorphicAssignment(prop2, prop, str1);
  }

  void TestColorProperty_Success()
  {
    mitk::Color c1;
    c1[0] = 0.2;
    c1[1] = 0.6;
    c1[2] = 0.8;
    mitk::Color c2;
    c2[0] = 0.2;
    c2[1] = 0.4;
    c2[2] = 0.1;
    TestProperty<mitk::ColorProperty>(c1, c2, "0.2  0.6  0.8", "0.2  0.4  0.1");
  }

  void TestLevelWindowProperty_Success()
  {
    mitk::LevelWindow lw1(50, 100);
    mitk::LevelWindow lw2(120, 30);
    TestProperty<mitk::LevelWindowProperty>(lw1, lw2, "L:50 W:100", "L:120 W:30");
  }

  void TestSmartPointerProperty_Success()
  {
    itk::Object::Pointer sp1 = itk::Object::New();
    itk::Object::Pointer sp2 = itk::Object::New();
    // to generate the UIDs, we set the smartpointers
    mitk::SmartPointerProperty::Pointer spp1 = mitk::SmartPointerProperty::New(sp1.GetPointer());
    mitk::SmartPointerProperty::Pointer spp2 = mitk::SmartPointerProperty::New(sp2.GetPointer());

    TestProperty<mitk::SmartPointerProperty>(sp1, sp2, spp1->GetReferenceUIDFor(sp1), spp2->GetReferenceUIDFor(sp2));
  }

  void TestStringProperty_Success()
  {
    TestProperty<mitk::StringProperty>("1", "2", "1", "2");
  }

  void TestTransferFunctionProperty_Success()
  {
    mitk::TransferFunction::Pointer tf1 = mitk::TransferFunction::New();
    mitk::TransferFunction::Pointer tf2 = mitk::TransferFunction::New();
    tf2->AddScalarOpacityPoint(0.4, 0.8);
    std::stringstream ss;
    ss << tf1;
    std::string strTF1 = ss.str();
    ss.str("");
    ss << tf2;
    std::string strTF2 = ss.str();
    TestProperty<mitk::TransferFunctionProperty>(tf1, tf2, strTF1, strTF2);
  }

  void TestWeakPointerProperty_Success()
  {
    itk::Object::Pointer sp1 = itk::Object::New();
    itk::Object::Pointer sp2 = itk::Object::New();
    mitk::WeakPointerProperty::ValueType wp1 = sp1.GetPointer();
    mitk::WeakPointerProperty::ValueType wp2 = sp2.GetPointer();
    std::stringstream ss;
    ss << sp1.GetPointer();
    std::string str1 = ss.str();
    ss.str("");
    ss << sp2.GetPointer();
    std::string str2 = ss.str();

    TestProperty<mitk::WeakPointerProperty>(wp1, wp2, str1, str2);
  }

  void TestLookupTablePropertyProperty_Success()
  {
    mitk::LookupTable::Pointer lut1 = mitk::LookupTable::New();
    lut1->GetVtkLookupTable()->SetTableValue(0, 0.2, 0.3, 0.4);
    mitk::LookupTable::Pointer lut2 = mitk::LookupTable::New();
    lut2->GetVtkLookupTable()->SetTableValue(0, 0.2, 0.4, 0.4);
    std::stringstream ss;
    ss << lut1;
    std::string strLUT1 = ss.str();
    ss.str("");
    ss << lut2;
    std::string strLUT2 = ss.str();
    TestProperty<mitk::LookupTableProperty>(lut1, lut2, strLUT1, strLUT2);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkProperty)

