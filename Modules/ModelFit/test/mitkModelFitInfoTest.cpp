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

#include <iostream>
#include "mitkTestingMacros.h"
#include "mitkProperties.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkModelFitInfo.h"
#include "mitkModelFitConstants.h"
#include "mitkModelFitException.h"

mitk::DataNode::Pointer generateModelFitTestNode()
{
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetName("Param1");
  auto testImage = mitk::Image::New();
  node->SetData(testImage);
  testImage->SetProperty("modelfit.testEmpty", mitk::StringProperty::New(""));
  testImage->SetProperty("modelfit.testValid", mitk::StringProperty::New("test"));
  mitk::EnsureModelFitUID(node);

  testImage->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit1"));
  testImage->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName"));
  testImage->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
  testImage->SetProperty(mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("input UID"));

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

mitk::StandaloneDataStorage::Pointer generateModelFitTestStorage()
{
  mitk::StandaloneDataStorage::Pointer storage = mitk::StandaloneDataStorage::New();

  //create input node
  mitk::DataNode::Pointer inputNode = mitk::DataNode::New();
  inputNode->SetName("Input");
  mitk::Image::Pointer image = mitk::Image::New();
  inputNode->SetData(image);
  mitk::NodeUIDType inputUID = mitk::EnsureModelFitUID(inputNode);

  storage->Add(inputNode);

  mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
  parents->push_back(inputNode);

  //create first result for Fit1
  mitk::DataNode::Pointer node1 = mitk::DataNode::New();
  mitk::Image::Pointer paramImage = mitk::Image::New();
  node1->SetData(paramImage);
  node1->SetName("Param1");
  mitk::EnsureModelFitUID(node1);
  paramImage->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit1"));
  paramImage->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName1"));
  paramImage->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
  paramImage->SetProperty(mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(inputUID.c_str()));

  paramImage->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
  paramImage->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
  paramImage->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
  paramImage->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
  paramImage->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

  paramImage->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
  paramImage->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
  paramImage->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
  paramImage->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

  paramImage->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param1"));
  paramImage->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("b"));
  paramImage->SetProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER().c_str()));

  storage->Add(node1,parents);

  //create second result for Fit1
  mitk::DataNode::Pointer node2 = mitk::DataNode::New();
  node2->SetName("Param2");
  mitk::Image::Pointer paramImage2 = mitk::Image::New();
  node2->SetData(paramImage2);
  mitk::EnsureModelFitUID(node2);
  paramImage2->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit1"));
  paramImage2->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName1"));
  paramImage2->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
  paramImage2->SetProperty(mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(inputUID.c_str()));

  paramImage2->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
  paramImage2->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_1"));
  paramImage2->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
  paramImage2->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass"));
  paramImage2->SetProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), mitk::StringProperty::New("myX"));

  paramImage2->SetProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT().c_str()));
  paramImage2->SetProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("h"));
  paramImage2->SetProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT().c_str()));
  paramImage2->SetProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("kg"));

  paramImage2->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param2"));
  paramImage2->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("a"));
  paramImage2->SetProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER().c_str()));

  storage->Add(node2, parents);

  //create result for Fit 2
  mitk::DataNode::Pointer node3 = mitk::DataNode::New();
  node3->SetName("Param_Other");
  mitk::Image::Pointer paramImage3 = mitk::Image::New();
  node3->SetData(paramImage3);
  mitk::EnsureModelFitUID(node3);
  paramImage3->SetProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Fit2"));
  paramImage3->SetProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("MyName2"));
  paramImage3->SetProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New(mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED().c_str()));
  paramImage3->SetProperty(mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(inputUID.c_str()));

  paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModels"));
  paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("TestModel_2"));
  paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), mitk::StringProperty::New(""));
  paramImage3->SetProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), mitk::StringProperty::New("ModelClass_B"));

  paramImage3->SetProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), mitk::StringProperty::New("Param_Other"));
  paramImage3->SetProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), mitk::StringProperty::New("a"));

  storage->Add(node3, parents);

  return storage;
}

int mitkModelFitInfoTest(int  /*argc*/, char*[] /*argv[]*/)
{
	MITK_TEST_BEGIN("mitkModelFitTest")

	mitk::modelFit::Parameter::Pointer p = mitk::modelFit::Parameter::New();
	p->name = "p";
	mitk::modelFit::ModelFitInfo::Pointer fit = mitk::modelFit::ModelFitInfo::New();
	fit->AddParameter(p);
	MITK_TEST_CONDITION_REQUIRED(fit->GetParameters().size() == 1,
								 "Testing if AddParameter successfully adds a parameter.");

	mitk::modelFit::Parameter::ConstPointer resultParam = fit->GetParameter("test",
			mitk::modelFit::Parameter::ParameterType);
	MITK_TEST_CONDITION_REQUIRED(resultParam.IsNull(),
								 "Testing if GetParameter returns NULL for wrong parameter.");

	resultParam = fit->GetParameter("p", mitk::modelFit::Parameter::ParameterType);
	MITK_TEST_CONDITION_REQUIRED(resultParam == p,
								 "Testing if GetParameter returns the correct parameter.");

	p->type = mitk::modelFit::Parameter::CriterionType;
	resultParam = fit->GetParameter("p", mitk::modelFit::Parameter::CriterionType);
	MITK_TEST_CONDITION_REQUIRED(resultParam == p,
								 "Testing if GetParameter returns the correct parameter with a " <<
								 "non-default type.");

	fit->DeleteParameter("test", mitk::modelFit::Parameter::CriterionType);
	MITK_TEST_CONDITION_REQUIRED(fit->GetParameters().size() == 1,
								 "Testing if DeleteParameter fails for wrong parameter.");

	fit->DeleteParameter("p", mitk::modelFit::Parameter::CriterionType);
	MITK_TEST_CONDITION_REQUIRED(fit->GetParameters().size() == 0,
								 "Testing if DeleteParameter successfully removes a parameter.");

	mitk::DataNode::Pointer node = generateModelFitTestNode();
  mitk::DataNode::Pointer invalideNode = mitk::DataNode::New();

  MITK_TEST_FOR_EXCEPTION(mitk::modelFit::ModelFitException,
							mitk::modelFit::GetMandatoryProperty(node.GetPointer(), "modelfit.testInvalid"));
  MITK_TEST_FOR_EXCEPTION(mitk::modelFit::ModelFitException,
    mitk::modelFit::GetMandatoryProperty(node.GetPointer(), "modelfit.testEmpty"));

	MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::GetMandatoryProperty(node.GetPointer(), "modelfit.testValid")
								 == "test",
								 "Testing if GetMandatoryProperty returns the correct value.");

  mitk::StandaloneDataStorage::Pointer storage = generateModelFitTestStorage();

  MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::CreateFitInfoFromNode("Fit1",NULL).IsNull(),
								 "Testing if CreateFitInfoFromNode returns NULL for invalid node.");

	MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::CreateFitInfoFromNode("invalide_UID",storage).IsNull(),
								 "Testing if CreateFitInfoFromNode returns NULL for node with " <<
								 "missing properties.");

  mitk::DataNode::Pointer testNode = storage->GetNamedNode("Param1");
	mitk::modelFit::ModelFitInfo::Pointer resultFit = mitk::modelFit::CreateFitInfoFromNode("Fit1", storage);
  MITK_TEST_CONDITION_REQUIRED(resultFit.IsNotNull(),
    "Testing if CreateFitInfoFromNode returns a valid model fit info.");
  MITK_TEST_CONDITION_REQUIRED(resultFit->fitType == mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED() &&
    resultFit->uid == "Fit1" &&
    resultFit->fitName == "MyName1" &&
    resultFit->modelType == "TestModels" &&
    resultFit->modelName == "TestModel_1" &&
    resultFit->function == ""&&
    resultFit->functionClassID == "ModelClass" &&
    resultFit->x == "myX" &&
    resultFit->xAxisName == mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT() &&
    resultFit->xAxisUnit == "h" &&
    resultFit->yAxisName == mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT() &&
    resultFit->yAxisUnit == "kg" &&
    resultFit->GetParameters().size() == 2,
    "Testing if CreateFitInfoFromNode creates a fit with correct attributes.");

  mitk::modelFit::Parameter::ConstPointer param = resultFit->GetParameter("Param1",mitk::modelFit::Parameter::ParameterType);
  MITK_TEST_CONDITION_REQUIRED(param.IsNotNull(),
    "Testing if param 1 exists.");

  MITK_TEST_CONDITION_REQUIRED(param->name == "Param1" && param->unit == "b" && param->image == testNode->GetData(),
    "Testing if param 1 is configured correctly.");

  mitk::modelFit::Parameter::ConstPointer param2 = resultFit->GetParameter("Param2",mitk::modelFit::Parameter::DerivedType);
  MITK_TEST_CONDITION_REQUIRED(param2.IsNotNull(),
    "Testing if param 2 exists.");

  testNode = storage->GetNamedNode("Param2");
  MITK_TEST_CONDITION_REQUIRED(param2->name == "Param2" && param2->unit == "a" && param2->image == testNode->GetData(),
    "Testing if param 2 is configured correctly.");

  MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::GetNodesOfFit(resultFit->uid,storage)->Size() == 2,
    "Testing if GetNodesOfFit works correctly for Fit1");

  MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::GetNodesOfFit("Fit2",storage)->Size() == 1,
    "Testing if GetNodesOfFit works correctly for Fit1");

  MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::GetNodesOfFit("unkown_fit",storage)->Size() == 0,
    "Testing if GetNodesOfFit works correctly for unkown fits.");

  MITK_TEST_CONDITION_REQUIRED(mitk::modelFit::GetNodesOfFit("unkown_fit",NULL).IsNull(),
    "Testing if GetNodesOfFit works correctly for illegal calls.");

  testNode = storage->GetNamedNode("Input");
  mitk::modelFit::NodeUIDSetType uidSet = mitk::modelFit::GetFitUIDsOfNode(testNode,storage);

  MITK_TEST_CONDITION_REQUIRED(uidSet.size() == 2 &&
    uidSet.find("Fit1")!=uidSet.end() &&
    uidSet.find("Fit2")!=uidSet.end(),
    "Testing if GetFitUIDsOfNode works correctly.");

  uidSet = mitk::modelFit::GetFitUIDsOfNode(NULL,storage);

  MITK_TEST_CONDITION_REQUIRED(uidSet.size() == 0,
    "Testing if GetFitUIDsOfNode works correctly with invalid node.");

  uidSet = mitk::modelFit::GetFitUIDsOfNode(testNode,NULL);

  MITK_TEST_CONDITION_REQUIRED(uidSet.size() == 0,
    "Testing if GetFitUIDsOfNode works correctly with invalid storage.");

  MITK_TEST_END()
}
