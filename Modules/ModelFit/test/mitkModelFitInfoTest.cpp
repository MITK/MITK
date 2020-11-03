/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>

#include "mitkProperties.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkModelFitInfo.h"
#include "mitkModelFitConstants.h"
#include "mitkModelFitException.h"
#include "mitkModelFitResultRelationRule.h"
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>


mitk::modelFit::ModelFitInfo::UIDType ensureModelFitUID(mitk::BaseData * data)
{
  mitk::BaseProperty::Pointer uidProp = data->GetProperty(mitk::ModelFitConstants::LEGACY_UID_PROPERTY_NAME().c_str());
  std::string propUID = "";

  if (uidProp.IsNotNull())
  {
    propUID = uidProp->GetValueAsString();
  }
  else
  {
    mitk::UIDGenerator generator;
    propUID = generator.GetUID();

    data->SetProperty(mitk::ModelFitConstants::LEGACY_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(propUID));
  }
  return propUID;
};

mitk::DataNode::Pointer generateModelFitTestNode()
{
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetName("Param1");
  auto testImage = mitk::Image::New();
  node->SetData(testImage);
  testImage->SetProperty("modelfit.testEmpty", mitk::StringProperty::New(""));
  testImage->SetProperty("modelfit.testValid", mitk::StringProperty::New("test"));
  ensureModelFitUID(testImage);

  testImage->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("FitLegacy"));
  testImage->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName"));
  testImage->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
  testImage->SetProperty(mitk::ModelFitConstants::LEGACY_FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("input UID"));

  testImage->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
  testImage->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
  testImage->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
  testImage->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
  testImage->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

  testImage->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
  testImage->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
  testImage->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
  testImage->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

  return node;
}

class mitkModelFitInfoTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkModelFitInfoTestSuite);
  MITK_TEST(CheckModelFitInfo);
  MITK_TEST(CheckGetMandatoryProperty);
  MITK_TEST(CheckCreateFitInfoFromNode_Legacy);
  MITK_TEST(CheckCreateFitInfoFromNode);
  MITK_TEST(CheckGetNodesOfFit);
  MITK_TEST(CheckGetFitUIDsOfNode);

  CPPUNIT_TEST_SUITE_END();

  mitk::StandaloneDataStorage::Pointer m_Storage;
  mitk::Image::Pointer m_ParamImage;
  mitk::Image::Pointer m_ParamImage2;
  mitk::Image::Pointer m_ParamImage_legacy;
  mitk::Image::Pointer m_ParamImage2_legacy;
  mitk::DataNode::Pointer m_ParamImageNode;
  mitk::DataNode::Pointer m_ParamImageNode2;
  mitk::DataNode::Pointer m_ParamImageNode_legacy;
  mitk::DataNode::Pointer m_ParamImageNode2_legacy;

public:
  void setUp() override
  {
    m_Storage = mitk::StandaloneDataStorage::New();
    //create input node
    mitk::DataNode::Pointer inputNode = mitk::DataNode::New();
    inputNode->SetName("Input");
    mitk::Image::Pointer image = mitk::Image::New();
    inputNode->SetData(image);
    mitk::modelFit::ModelFitInfo::UIDType inputUID = ensureModelFitUID(image);

    m_Storage->Add(inputNode);

    mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
    parents->push_back(inputNode);

    /////////////////////////////////////////////////////
    //Create nodes for a fit (new style using rules)
    /////////////////////////////////////////////////////
    auto rule = mitk::ModelFitResultRelationRule::New();
    //create first result for FitLegacy
    m_ParamImageNode = mitk::DataNode::New();
    m_ParamImage = mitk::Image::New();
    m_ParamImageNode->SetData(m_ParamImage);
    m_ParamImageNode->SetName("Param1");
    m_ParamImage->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName1"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));

    m_ParamImage->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

    m_ParamImage->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

    m_ParamImage->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param1"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("b"));
    m_ParamImage->SetProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER().c_str()));
    rule->Connect(m_ParamImage, image);
    m_Storage->Add(m_ParamImageNode, parents);

    //create second result for Fit
    m_ParamImageNode2 = mitk::DataNode::New();
    m_ParamImageNode2->SetName("Param2");
    m_ParamImage2 = mitk::Image::New();
    m_ParamImageNode2->SetData(m_ParamImage2);
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName1"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));

    m_ParamImage2->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

    m_ParamImage2->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

    m_ParamImage2->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param2"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("a"));
    m_ParamImage2->SetProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER().c_str()));
    rule->Connect(m_ParamImage2, image);
    m_Storage->Add(m_ParamImageNode2, parents);

    /////////////////////////////////////////////////////
    //Create nodes for a fit in legacy mode (old fit uid)
    /////////////////////////////////////////////////////

    //create first result for FitLegacy
    m_ParamImageNode_legacy = mitk::DataNode::New();
    m_ParamImage_legacy = mitk::Image::New();
    m_ParamImageNode_legacy->SetData(m_ParamImage_legacy);
    m_ParamImageNode_legacy->SetName("Param1_legacy");
    ensureModelFitUID(m_ParamImage_legacy);
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("FitLegacy"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName1"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::LEGACY_FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(inputUID.c_str()));

    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param1_legacy"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("b"));
    m_ParamImage_legacy->SetProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER().c_str()));

    m_Storage->Add(m_ParamImageNode_legacy, parents);

    //create second result for FitLegacy
    m_ParamImageNode2_legacy = mitk::DataNode::New();
    m_ParamImageNode2_legacy->SetName("Param2_legacy");
    m_ParamImage2_legacy = mitk::Image::New();
    m_ParamImageNode2_legacy->SetData(m_ParamImage2_legacy);
    ensureModelFitUID(m_ParamImage2_legacy);
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("FitLegacy"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName1"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::LEGACY_FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(inputUID.c_str()));

    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param2_legacy"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("a"));
    m_ParamImage2_legacy->SetProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER().c_str()));

    m_Storage->Add(m_ParamImageNode2_legacy, parents);

    /////////////////////////////////////////////////////
    //Create nodes for a fit on other input
    auto anotherInputNode = mitk::DataNode::New();
    anotherInputNode->SetName("AnotherInput");
    auto anotherImage = mitk::Image::New();
    anotherInputNode->SetData(anotherImage);

    m_Storage->Add(anotherInputNode);

    parents = mitk::DataStorage::SetOfObjects::New();
    parents->push_back(anotherInputNode);

    mitk::DataNode::Pointer node3 = mitk::DataNode::New();
    node3->SetName("Param_Other");
    mitk::Image::Pointer paramImage3 = mitk::Image::New();
    node3->SetData(paramImage3);
    paramImage3->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit2"));
    paramImage3->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName2"));
    paramImage3->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));

    paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
    paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_2"));
    paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
    paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass_B"));

    paramImage3->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param_Other"));
    paramImage3->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("a"));
    rule->Connect(paramImage3, anotherImage);
    m_Storage->Add(node3, parents);

  }

  void tearDown() override
  {
  }

  void CheckModelFitInfo()
  {
    mitk::modelFit::Parameter::Pointer p = mitk::modelFit::Parameter::New();
    p->name = "p";
    mitk::modelFit::ModelFitInfo::Pointer fit = mitk::modelFit::ModelFitInfo::New();
    fit->AddParameter(p);
    CPPUNIT_ASSERT_MESSAGE("AddParameter unsuccessfully adds a parameter.", fit->GetParameters().size() == 1);

    mitk::modelFit::Parameter::ConstPointer resultParam = fit->GetParameter("test",
      mitk::modelFit::Parameter::ParameterType);
    CPPUNIT_ASSERT_MESSAGE("Testing if GetParameter returns NULL for wrong parameter.", resultParam.IsNull());

    resultParam = fit->GetParameter("p", mitk::modelFit::Parameter::ParameterType);
    CPPUNIT_ASSERT_MESSAGE("Testing if GetParameter returns the correct parameter.", resultParam == p);

    p->type = mitk::modelFit::Parameter::CriterionType;
    resultParam = fit->GetParameter("p", mitk::modelFit::Parameter::CriterionType);
    CPPUNIT_ASSERT_MESSAGE("Testing if GetParameter returns the correct parameter with a non-default type.", resultParam == p);

    fit->DeleteParameter("test", mitk::modelFit::Parameter::CriterionType);
    CPPUNIT_ASSERT_MESSAGE("Testing if DeleteParameter fails for wrong parameter.", fit->GetParameters().size() == 1);

    fit->DeleteParameter("p", mitk::modelFit::Parameter::CriterionType);
    CPPUNIT_ASSERT_MESSAGE("Testing if DeleteParameter successfully removes a parameter.", fit->GetParameters().size() == 0);
  }

  void CheckGetMandatoryProperty()
  {
    mitk::DataNode::Pointer node = generateModelFitTestNode();
    mitk::DataNode::Pointer invalideNode = mitk::DataNode::New();

    CPPUNIT_ASSERT_THROW(mitk::modelFit::GetMandatoryProperty(node.GetPointer(), "modelfit.testInvalid"), mitk::modelFit::ModelFitException);
    CPPUNIT_ASSERT_THROW(mitk::modelFit::GetMandatoryProperty(node.GetPointer(), "modelfit.testEmpty"), mitk::modelFit::ModelFitException);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetMandatoryProperty returns the correct value.", mitk::modelFit::GetMandatoryProperty(node.GetPointer(), "modelfit.testValid")
      == "test");
  }

  void CheckCreateFitInfoFromNode_Legacy()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode returns NULL for invalid node.", mitk::modelFit::CreateFitInfoFromNode("FitLegacy", nullptr).IsNull());

    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode returns NULL for node with missing properties.", mitk::modelFit::CreateFitInfoFromNode("invalide_UID", m_Storage).IsNull());

    mitk::modelFit::ModelFitInfo::Pointer resultFit = mitk::modelFit::CreateFitInfoFromNode("FitLegacy", m_Storage);
    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode returns a valid model fit info.", resultFit.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode creates a fit with correct attributes.",
      resultFit->fitType == mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED() &&
      resultFit->uid == "FitLegacy" &&
      resultFit->fitName == "MyName1" &&
      resultFit->modelType == "TestModels" &&
      resultFit->modelName == "TestModel_1" &&
      resultFit->function == "" &&
      resultFit->functionClassID == "ModelClass" &&
      resultFit->x == "myX" &&
      resultFit->xAxisName == mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT() &&
      resultFit->xAxisUnit == "h" &&
      resultFit->yAxisName == mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT() &&
      resultFit->yAxisUnit == "kg" &&
      resultFit->GetParameters().size() == 2);

    mitk::modelFit::Parameter::ConstPointer param = resultFit->GetParameter("Param1_legacy", mitk::modelFit::Parameter::ParameterType);
    CPPUNIT_ASSERT_MESSAGE("Testing if param 1 exists.", param.IsNotNull());

    CPPUNIT_ASSERT_MESSAGE("Testing if param 1 is configured correctly.", param->name == "Param1_legacy" && param->unit == "b" && param->image == m_ParamImage_legacy);

    mitk::modelFit::Parameter::ConstPointer param2 = resultFit->GetParameter("Param2_legacy", mitk::modelFit::Parameter::DerivedType);
    CPPUNIT_ASSERT_MESSAGE("Testing if param 2 exists.", param2.IsNotNull());

    CPPUNIT_ASSERT_MESSAGE("Testing if param 2 is configured correctly.", param2->name == "Param2_legacy" && param2->unit == "a" && param2->image == m_ParamImage2_legacy);
  }

  void CheckCreateFitInfoFromNode()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode returns NULL for invalid node.", mitk::modelFit::CreateFitInfoFromNode("Fit", nullptr).IsNull());

    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode returns NULL for node with missing properties.", mitk::modelFit::CreateFitInfoFromNode("invalide_UID", m_Storage).IsNull());

    mitk::modelFit::ModelFitInfo::Pointer resultFit = mitk::modelFit::CreateFitInfoFromNode("Fit", m_Storage);
    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode returns a valid model fit info.", resultFit.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Testing if CreateFitInfoFromNode creates a fit with correct attributes.",
      resultFit->fitType == mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED() &&
      resultFit->uid == "Fit" &&
      resultFit->fitName == "MyName1" &&
      resultFit->modelType == "TestModels" &&
      resultFit->modelName == "TestModel_1" &&
      resultFit->function == "" &&
      resultFit->functionClassID == "ModelClass" &&
      resultFit->x == "myX" &&
      resultFit->xAxisName == mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT() &&
      resultFit->xAxisUnit == "h" &&
      resultFit->yAxisName == mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT() &&
      resultFit->yAxisUnit == "kg" &&
      resultFit->GetParameters().size() == 2);

    mitk::modelFit::Parameter::ConstPointer param = resultFit->GetParameter("Param1", mitk::modelFit::Parameter::ParameterType);
    CPPUNIT_ASSERT_MESSAGE("Testing if param 1 exists.", param.IsNotNull());

    CPPUNIT_ASSERT_MESSAGE("Testing if param 1 is configured correctly.", param->name == "Param1" && param->unit == "b" && param->image == m_ParamImage);

    mitk::modelFit::Parameter::ConstPointer param2 = resultFit->GetParameter("Param2", mitk::modelFit::Parameter::DerivedType);
    CPPUNIT_ASSERT_MESSAGE("Testing if param 2 exists.", param2.IsNotNull());

    CPPUNIT_ASSERT_MESSAGE("Testing if param 2 is configured correctly.", param2->name == "Param2" && param2->unit == "a" && param2->image == m_ParamImage2);
  }

  void CheckGetNodesOfFit()
  {
    auto nodes = mitk::modelFit::GetNodesOfFit("Fit", m_Storage);
    CPPUNIT_ASSERT_MESSAGE("Testing if GetNodesOfFit works correctly for Fit",
      nodes->Size() == 2);
    CPPUNIT_ASSERT(std::find(nodes->begin(), nodes->end(), m_ParamImageNode.GetPointer()) != nodes->end());
    CPPUNIT_ASSERT(std::find(nodes->begin(), nodes->end(), m_ParamImageNode2.GetPointer()) != nodes->end());

    nodes = mitk::modelFit::GetNodesOfFit("FitLegacy", m_Storage);
    CPPUNIT_ASSERT_MESSAGE("Testing if GetNodesOfFit works correctly for FitLegacy",
      nodes->Size() == 2);
    CPPUNIT_ASSERT(std::find(nodes->begin(), nodes->end(), m_ParamImageNode_legacy.GetPointer()) != nodes->end());
    CPPUNIT_ASSERT(std::find(nodes->begin(), nodes->end(), m_ParamImageNode2_legacy.GetPointer()) != nodes->end());

    CPPUNIT_ASSERT_MESSAGE("Testing if GetNodesOfFit works correctly for Fit2",
      mitk::modelFit::GetNodesOfFit("Fit2", m_Storage)->Size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetNodesOfFit works correctly for unkown fits.",
      mitk::modelFit::GetNodesOfFit("unkown_fit", m_Storage)->Size() == 0);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetNodesOfFit works correctly for illegal calls.",
      mitk::modelFit::GetNodesOfFit("unkown_fit", nullptr).IsNull());
  }

  void CheckGetFitUIDsOfNode()
  {
    auto testNode = m_Storage->GetNamedNode("Input");
    mitk::modelFit::NodeUIDSetType uidSet = mitk::modelFit::GetFitUIDsOfNode(testNode, m_Storage);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetFitUIDsOfNode works correctly.",
      uidSet.size() == 2 &&
      uidSet.find("Fit") != uidSet.end() &&
      uidSet.find("FitLegacy") != uidSet.end());

    testNode = m_Storage->GetNamedNode("AnotherInput");
    uidSet = mitk::modelFit::GetFitUIDsOfNode(testNode, m_Storage);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetFitUIDsOfNode works correctly.",
      uidSet.size() == 1 &&
      uidSet.find("Fit2") != uidSet.end());

    uidSet = mitk::modelFit::GetFitUIDsOfNode(nullptr, m_Storage);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetFitUIDsOfNode works correctly with invalid node.",
      uidSet.size() == 0);

    uidSet = mitk::modelFit::GetFitUIDsOfNode(testNode, nullptr);

    CPPUNIT_ASSERT_MESSAGE("Testing if GetFitUIDsOfNode works correctly with invalid storage.",
      uidSet.size() == 0);
  }
};


MITK_TEST_SUITE_REGISTRATION(mitkModelFitInfo)