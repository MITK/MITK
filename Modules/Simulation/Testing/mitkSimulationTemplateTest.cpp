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

#include <mitkSimulationObjectFactory.h>
#include <mitkSimulationTemplate.h>
#include <mitkTestingMacros.h>

#define _MITK_TEST_FOR_EXCEPTION(STATEMENT, EXCEPTION, MESSAGE) \
  MITK_TEST_OUTPUT_NO_ENDL(<< MESSAGE) \
  try \
  { \
    STATEMENT; \
    MITK_TEST_OUTPUT(<< " [FAILED]") \
    mitk::TestManager::GetInstance()->TestFailed(); \
  } \
  catch (const EXCEPTION& e) \
  { \
    MITK_TEST_OUTPUT(<< "\n  " << e.GetDescription() << " [PASSED]") \
    mitk::TestManager::GetInstance()->TestPassed(); \
  }

static mitk::DataNode::Pointer CreateDataNode(mitk::SimulationTemplate::Pointer simulationTemplate, bool setProperties = false)
{
  if (simulationTemplate.IsNull())
    mitkThrow() << "Invalid argument (null pointer!)";

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  dataNode->SetData(simulationTemplate);

  if (setProperties)
    simulationTemplate->SetProperties(dataNode);

  return dataNode;
}

static void Parse_InputIsEmpty_ReturnsTrue()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(simulationTemplate->Parse(""), "Parse_InputIsEmpty_ReturnsTrue")
}

static void Parse_AlreadyInitialized_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("");
  MITK_TEST_CONDITION(!simulationTemplate->Parse(""), "Parse_AlreadyInitialized_ReturnsFalse")
}

static void Parse_EOFBeforeClosingBrace_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='A' default='1'"), mitk::Exception, "Parse_EOFBeforeClosingBrace_ThrowsException")
}

static void Parse_OpeningBraceBeforeClosingBrace_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='A' default='1' {id='B' default='2'}}"), mitk::Exception, "Parse_OpeningBraceBeforeClosingBrace_ThrowsException")
}

static void Parse_TooShortToBeReference_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{}"), mitk::Exception, "Parse_TooShortToBeReference_ThrowsException")
}

static void Parse_NumberOfSingleQuotationMarksIsOdd_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{'Flinders's cat'}"), mitk::Exception, "Parse_NumberOfSingleQuotationMarksIsOdd_ThrowsException")
}

static void Parse_ReferenceDoesNotEndCorrectly_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{'Flinders\\'s cat}"), mitk::Exception, "Parse_ReferenceDoesNotEndCorrectly_ThrowsException")
}

static void Parse_ReferenceIsEmpty_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{''}"), mitk::Exception, "Parse_ReferenceIsEmpty_ThrowsException")
}

static void Parse_TooShortToBeTemplate_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id=}"), mitk::Exception, "Parse_TooShortToBeTemplate_ThrowsException")
}

static void Parse_IdIsEmpty_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id=''}"), mitk::Exception, "Parse_IdIsEmpty_ThrowsException")
}

static void Parse_UnknownType_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='Trim' type='Cat'}"), mitk::Exception, "Parse_UnknownType_ThrowsException")
}

static void Parse_ValueIsAmbiguous_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='Matthew' id='Flinders'}"), mitk::Exception, "Parse_ValueIsAmbiguous_ThrowsException")
}

static void Parse_IdNotFound_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{type='int' value='42'}"), mitk::Exception, "Parse_IdNotFound_ThrowsException")
}

static void Parse_NoAssignmentToValue_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id default=''}"), mitk::Exception, "Parse_NoAssignmentToValue_ThrowsException")
}

static void Parse_ValueIsNotEnclosedWithSingleQuotationMarks_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id=Flinders\\'s Cat}"), mitk::Exception, "Parse_ValueIsNotEnclosedWithSingleQuotationMarks_ThrowsException")
}

static void Parse_TypeMismatch_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='Answer' type='int' default='forty-two'}"), mitk::Exception, "Parse_TypeMismatch_ThrowsException")
}

static void Parse_ValueContainsEscapedSingleQuotationMark_IsUnescapedAfterParsing()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Flinders\\'s Cat' default='Trim'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  std::string value;
  dataNode->GetStringProperty("Flinders's Cat", value);
  MITK_TEST_CONDITION(value == "Trim", "Parse_ValueContainsEscapedSingleQuotationMark_IsUnescapedAfterParsing")
}

static void Parse_Float_ParsedAsFloatProperty()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Pi' type='float' default='3.14'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  float value;
  MITK_TEST_CONDITION(dataNode->GetFloatProperty("Pi", value) && mitk::Equal(value, 3.14f), "Parse_Float_ParsedAsFloatProperty")
}

static void Parse_Int_ParsedAsIntProperty()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Answer' type='int' default='42'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  int value;
  MITK_TEST_CONDITION(dataNode->GetIntProperty("Answer", value) && value == 42, "Parse_Int_ParsedAsIntProperty")
}

static void Parse_String_ParsedAsStringProperty()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Cat' type='string' default='Trim'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  std::string value;
  MITK_TEST_CONDITION(dataNode->GetStringProperty("Cat", value) && value == "Trim", "Parse_String_ParsedAsStringProperty")
}

static void Parse_FloatDefaultValue_IsZero()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Zero' type='float'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  float value;
  MITK_TEST_CONDITION(dataNode->GetFloatProperty("Zero", value) && mitk::Equal(value, 0.0f), "Parse_FloatDefaultValue_IsZero")
}

static void Parse_IntDefaultValue_IsZero()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Zero' type='int'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  int value;
  MITK_TEST_CONDITION(dataNode->GetIntProperty("Zero", value) && value == 0, "Parse_IntDefaultValue_IsZero")
}

static void Parse_StringDefaultValue_IsEmpty()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Empty' type='string'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  std::string value;
  MITK_TEST_CONDITION(dataNode->GetStringProperty("Empty", value) && value.empty(), "Parse_StringDefaultValue_IsEmpty")
}

static void Parse_NoType_ParsedAsString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Captain' default='Flinders'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate, true);
  std::string value;
  MITK_TEST_CONDITION(dataNode->GetStringProperty("Captain", value) && value == "Flinders", "Parse_NoType_ParsedAsString")
}

static void SetProperties_InputIsNull_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(!simulationTemplate->SetProperties(NULL), "SetProperties_InputIsNull_ReturnsFalse")
}

static void SetProperties_NotInitialized_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate);
  MITK_TEST_CONDITION(!simulationTemplate->SetProperties(dataNode), "SetProperties_NotInitialized_ReturnsFalse")
}

static void SetProperties_InputIsNotOwner_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate1 = mitk::SimulationTemplate::New();
  simulationTemplate1->Parse("{id='A' default='1'}");
  mitk::SimulationTemplate::Pointer simulationTemplate2 = mitk::SimulationTemplate::New();
  simulationTemplate2->Parse("{id='B' default='2'}");
  mitk::DataNode::Pointer dataNode1 = CreateDataNode(simulationTemplate1);
  MITK_TEST_CONDITION(!simulationTemplate2->SetProperties(dataNode1), "SetProperties_InputIsNotOwner_ReturnsFalse")
}

static void SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Atoll' default='Bora'} {'Atoll'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate);
  bool boolResult = simulationTemplate->SetProperties(dataNode);
  std::size_t numProperties = dataNode->GetPropertyList()->GetMap()->size();
  std::string stringResult;
  dataNode->GetStringProperty("Atoll", stringResult);
  MITK_TEST_CONDITION(boolResult && numProperties == 1 && stringResult == "Bora", "SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue")
}

static void CreateSimulation_NotInitialized_ReturnsEmptyString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation().empty(), "CreateSimulation_NotInitialized_ReturnsEmptyString")
}

static void CreateSimulation_ContainsInvalidReference_ReturnsEmptyString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{'Invalid'}");
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation().empty(), "CreateSimulation_ContainsInvalidReference_ReturnsEmptyString")
}

static void CreateSimulation_ContainsTemplateAndReference_ReturnsExpectedString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Atoll' default='Bora'} {'Atoll'}");
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation() == "Bora Bora", "CreateSimulation_ContainsTemplateAndReference_ReturnsExpectedString")
}

int mitkSimulationTemplateTest(int, char* [])
{
  mitk::RegisterSimulationObjectFactory();

  MITK_TEST_BEGIN("mitkSimulationTemplateTest")

    Parse_InputIsEmpty_ReturnsTrue();
    Parse_AlreadyInitialized_ReturnsFalse();
    Parse_EOFBeforeClosingBrace_ThrowsException();
    Parse_OpeningBraceBeforeClosingBrace_ThrowsException();
    Parse_TooShortToBeReference_ThrowsException();
    Parse_NumberOfSingleQuotationMarksIsOdd_ThrowsException();
    Parse_ReferenceDoesNotEndCorrectly_ThrowsException();
    Parse_ReferenceIsEmpty_ThrowsException();
    Parse_TooShortToBeTemplate_ThrowsException();
    Parse_IdIsEmpty_ThrowsException();
    Parse_UnknownType_ThrowsException();
    Parse_ValueIsAmbiguous_ThrowsException();
    Parse_IdNotFound_ThrowsException();
    Parse_NoAssignmentToValue_ThrowsException();
    Parse_ValueIsNotEnclosedWithSingleQuotationMarks_ThrowsException();
    Parse_TypeMismatch_ThrowsException();
    Parse_ValueContainsEscapedSingleQuotationMark_IsUnescapedAfterParsing();
    Parse_Float_ParsedAsFloatProperty();
    Parse_Int_ParsedAsIntProperty();
    Parse_String_ParsedAsStringProperty();
    Parse_FloatDefaultValue_IsZero();
    Parse_IntDefaultValue_IsZero();
    Parse_StringDefaultValue_IsEmpty();
    Parse_NoType_ParsedAsString();

    SetProperties_InputIsNull_ReturnsFalse();
    SetProperties_NotInitialized_ReturnsFalse();
    SetProperties_InputIsNotOwner_ReturnsFalse();
    SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue();

    CreateSimulation_NotInitialized_ReturnsEmptyString();
    CreateSimulation_ContainsInvalidReference_ReturnsEmptyString();
    CreateSimulation_ContainsTemplateAndReference_ReturnsExpectedString();

  MITK_TEST_END()
}
