/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabel.h"

#include "itkProcessObject.h"
#include <itkCommand.h>
#include <mitkProperties.h>
#include <mitkDICOMSegmentationConstants.h>
#include <mitkStringProperty.h>

#include <regex>

const mitk::Label::PixelType mitk::Label::MAX_LABEL_VALUE = std::numeric_limits<mitk::Label::PixelType>::max();

namespace mitk
{
  namespace LabelPropertyConstants
  {
    const std::string& GetAnatomicRegionPropertyBaseName()
    {
      static const std::string name = "anatomic_region";
      return name;
    }

    const std::string& GetPrimaryAnatomicStructurePropertyBaseName()
    {
      static const std::string name = "primary_anatomic_structure";
      return name;
    }

    const std::string& GetSegmentedPropertyCategoryPropertyBaseName()
    {
      static const std::string name = "segmented_property_category";
      return name;
    }

    const std::string& GetSegmentedPropertyTypePropertyBaseName()
    {
      static const std::string name = "segmented_property_type";
      return name;
    }

    const std::string& GetModifierPropertySubName()
    {
      static const std::string name = "modifier";
      return name;
    }

    const std::string& GetValuePropertySubName()
    {
      static const std::string name = "value";
      return name;
    }

    const std::string& GetSchemePropertySubName()
    {
      static const std::string name = "scheme";
      return name;
    }

    const std::string& GetMeaningPropertySubName()
    {
      static const std::string name = "meaning";
      return name;
    }
  }
} // namespace mitk



mitk::Label::Label() : PropertyList(), m_Value(UNLABELED_VALUE)
{
  if (GetProperty("locked") == nullptr)
    SetLocked(true);
  if (GetProperty("visible") == nullptr)
    SetVisible(true);
  if (GetProperty("opacity") == nullptr)
    SetOpacity(0.6);

  if (GetProperty("center.coordinates") == nullptr)
  {
    mitk::Point3D pnt;
    pnt.SetElement(0, 0);
    pnt.SetElement(1, 0);
    pnt.SetElement(2, 0);
    SetCenterOfMassCoordinates(pnt);
  }
  if (GetProperty("center.index") == nullptr)
  {
    mitk::Point3D pnt;
    pnt.SetElement(0, 0);
    pnt.SetElement(1, 0);
    pnt.SetElement(2, 0);
    SetCenterOfMassIndex(pnt);
  }
  if (GetProperty("color") == nullptr)
  {
    mitk::Color col;
    col.Set(1, 1, 1);
    SetColor(col);
  }

  if (GetProperty("name") == nullptr)
    SetName("Unknown label name");

  if (GetProperty("description") == nullptr)
    SetDescription("");
}

mitk::Label::Label(PixelType value, const std::string& name) : Label()
{
  this->SetValue(value);
  this->SetName(name);
}

mitk::Label::Label(const Label &other) : PropertyList(other), m_Value(other.m_Value)
// copy constructor of property List handles the coping action
{
  auto *map = this->GetMap();
  auto it = map->begin();
  auto end = map->end();

  for (; it != end; ++it)
  {
    itk::SimpleMemberCommand<Label>::Pointer command = itk::SimpleMemberCommand<Label>::New();
    command->SetCallbackFunction(this, &Label::Modified);
    it->second->AddObserver(itk::ModifiedEvent(), command);
  }
}

mitk::Label::~Label()
{
}

void mitk::Label::SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName, bool fallBackOnDefaultContext)
{
  itk::SimpleMemberCommand<Label>::Pointer command = itk::SimpleMemberCommand<Label>::New();
  command->SetCallbackFunction(this, &Label::Modified);
  property->AddObserver(itk::ModifiedEvent(), command);

  Superclass::SetProperty(propertyKey, property, contextName, fallBackOnDefaultContext);
}

mitk::BaseProperty::ConstPointer mitk::Label::GetConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext) const
{
  return Superclass::GetConstProperty(propertyKey, contextName, fallBackOnDefaultContext);
}

mitk::BaseProperty* mitk::Label::GetNonConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext)
{
  return Superclass::GetNonConstProperty(propertyKey, contextName, fallBackOnDefaultContext);
}

void mitk::Label::SetLocked(bool locked)
{
  mitk::BoolProperty *property = dynamic_cast<mitk::BoolProperty *>(GetProperty("locked"));
  if (property != nullptr)
    // Update Property
    property->SetValue(locked);
  else
    // Create new Property
    SetBoolProperty("locked", locked);
}

bool mitk::Label::GetLocked() const
{
  bool locked;
  GetBoolProperty("locked", locked);
  return locked;
}

void mitk::Label::SetVisible(bool visible)
{
  mitk::BoolProperty *property = dynamic_cast<mitk::BoolProperty *>(GetProperty("visible"));
  if (property != nullptr)
    // Update Property
    property->SetValue(visible);
  else
    // Create new Property
    SetBoolProperty("visible", visible);
}

bool mitk::Label::GetVisible() const
{
  bool visible;
  GetBoolProperty("visible", visible);
  return visible;
}

void mitk::Label::SetOpacity(float opacity)
{
  mitk::FloatProperty *property = dynamic_cast<mitk::FloatProperty *>(GetProperty("opacity"));
  if (property != nullptr)
    // Update Property
    property->SetValue(opacity);
  else
    // Create new Property
    SetFloatProperty("opacity", opacity);
}

float mitk::Label::GetOpacity() const
{
  float opacity;
  GetFloatProperty("opacity", opacity);
  return opacity;
}

void mitk::Label::SetName(const std::string &name)
{
  SetStringProperty("name", name.c_str());
}

std::string mitk::Label::GetName() const
{
  std::string name;
  GetStringProperty("name", name);
  return name;
}

std::string mitk::Label::GetTrackingID() const
{
  std::string trackingID = "";
  GetStringProperty("tracking_id", trackingID);
  return trackingID;
}

void mitk::Label::SetTrackingID(const std::string& trackingID)
{
  mitk::StringProperty* property = dynamic_cast<mitk::StringProperty*>(GetProperty("tracking_id"));
  if (property != nullptr)
    // Update Property
    property->SetValue(trackingID);
  else
    // Create new Property
    SetStringProperty("tracking_id", trackingID.c_str());
}

std::string mitk::Label::GetTrackingUID() const
{
  std::string trackingUID = "";
  GetStringProperty("tracking_uid", trackingUID);
  return trackingUID;
}

void mitk::Label::SetTrackingUID(const std::string& trackingUID)
{
  mitk::StringProperty* property = dynamic_cast<mitk::StringProperty*>(GetProperty("tracking_uid"));
  if (property != nullptr)
    // Update Property
    property->SetValue(trackingUID);
  else
    // Create new Property
    SetStringProperty("tracking_uid", trackingUID.c_str());
}

std::string mitk::Label::GetDescription() const
{
  std::string description = "";
  GetStringProperty("description", description);
  return description;
}

void mitk::Label::SetDescription(const std::string& description)
{
  mitk::StringProperty* property = dynamic_cast<mitk::StringProperty*>(GetProperty("description"));
  if (property != nullptr)
    // Update Property
    property->SetValue(description);
  else
    // Create new Property
    SetStringProperty("description", description.c_str());
}

void mitk::Label::SetValue(PixelType pixelValue)
{
  mitk::UShortProperty *property = dynamic_cast<mitk::UShortProperty *>(GetProperty("value"));
  if (pixelValue != m_Value)
  {
    // Update Property
    m_Value = pixelValue;
    this->Modified();
  }
}

mitk::Label::PixelType mitk::Label::GetValue() const
{
  return m_Value;
}

const mitk::Color &mitk::Label::GetColor() const
{
  mitk::ColorProperty *colorProp = dynamic_cast<mitk::ColorProperty *>(GetProperty("color"));
  return colorProp->GetColor();
}

void mitk::Label::SetColor(const mitk::Color &_color)
{
  mitk::ColorProperty *colorProp = dynamic_cast<mitk::ColorProperty *>(GetProperty("color"));
  if (colorProp != nullptr)
    // Update Property
    colorProp->SetColor(_color);
  else
    // Create new Property
    SetProperty("color", mitk::ColorProperty::New(_color));
}

void mitk::Label::SetCenterOfMassIndex(const mitk::Point3D &center)
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.index"));
  if (property != nullptr)
    // Update Property
    property->SetValue(center);
  else
    // Create new Property
    SetProperty("center.index", mitk::Point3dProperty::New(center));
}

mitk::Point3D mitk::Label::GetCenterOfMassIndex() const
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.index"));
  return property->GetValue();
}

void mitk::Label::SetCenterOfMassCoordinates(const mitk::Point3D &center)
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.coordinates"));
  if (property != nullptr)
    // Update Property
    property->SetValue(center);
  else
    // Create new Property
    SetProperty("center.coordinates", mitk::Point3dProperty::New(center));
}

mitk::Point3D mitk::Label::GetCenterOfMassCoordinates() const
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.coordinates"));
  return property->GetValue();
}

void mitk::Label::SetAlgorithmType(AlgorithmType algoType)
{
  std::string text = "";
  if (algoType == AlgorithmType::MANUAL)
    text = "MANUAL";
  else if (algoType == AlgorithmType::SEMIAUTOMATIC)
    text = "SEMIAUTOMATIC";
  else if (algoType == AlgorithmType::AUTOMATIC)
    text = "AUTOMATIC";

  this->SetAlgorithmTypeStr(text);
}

void mitk::Label::SetAlgorithmTypeStr(const std::string& algoType)
{
  if (algoType.empty())
  {
    this->RemoveProperty("algorithm_type");
  }
  else if (algoType != "MANUAL" && algoType != "SEMIAUTOMATIC" && algoType != "AUTOMATIC")
  {
    mitkThrow() << "Invalid call of Label::SetAlgorithmTypeStr. Passed string is an unsupported type. Invalid string: "<<algoType;
  }
  else
  {
    mitk::StringProperty* property = dynamic_cast<mitk::StringProperty*>(this->GetProperty("algorithm_type"));
    if (property != nullptr)
      // Update Property
      property->SetValue(algoType);
    else
      // Create new Property
      SetStringProperty("algorithm_type", algoType.c_str());
  }
}

mitk::Label::AlgorithmType mitk::Label::GetAlgorithmType() const
{
  std::string text = this->GetAlgorithmTypeStr();

  if (text.empty())
    return AlgorithmType::Undefined;
  else if (text =="MANUAL")
    return AlgorithmType::MANUAL;
  else if (text == "SEMIAUTOMATIC")
    return AlgorithmType::SEMIAUTOMATIC;

  return AlgorithmType::AUTOMATIC;
}

std::string mitk::Label::GetAlgorithmTypeStr() const
{
  std::string text = "";
  GetStringProperty("algorithm_type", text);
  return text;
}

void mitk::Label::SetAlgorithmName(const std::string& algoName)
{
  mitk::StringProperty* property = dynamic_cast<mitk::StringProperty*>(this->GetProperty("algorithm_name"));
  if (property != nullptr)
    // Update Property
    property->SetValue(algoName);
  else
    // Create new Property
    SetStringProperty("algorithm_type", algoName.c_str());
}

std::string mitk::Label::GetAlgorithmName() const
{
  std::string text = "MITK Segmentation";
  GetStringProperty("algorithm_name", text);
  return text;
}

void mitk::Label::AddToolUse(AlgorithmType algoType, const std::string& algoName)
{
  auto currentType = this->GetAlgorithmType();
  auto currentName = this->GetAlgorithmName();

  if (currentType == AlgorithmType::Undefined)
    this->SetAlgorithmType(algoType);
  else if (currentType != algoType)
    this->SetAlgorithmType(AlgorithmType::SEMIAUTOMATIC);

  auto pos = currentName.find(algoName);
  if (pos != std::string::npos)
  {
    if (!currentName.empty())
      currentName += " | ";
    currentName += algoName;
    this->SetAlgorithmName(currentName);
  }
}


void mitk::Label::Update(const Label* templateLabel, bool updateLabelValue)
{
  if (nullptr == templateLabel)
    mitkThrow() << "Invalid call of Label::Update. Passed label is null.";

  auto keys = templateLabel->GetPropertyKeys();
  for (const auto& key : keys)
  {
    auto destProp = templateLabel->GetConstProperty(key)->Clone();
    this->SetProperty(key, destProp); //The implementation of SetProperty ensures
                                      //that only the content will be assigned if
                                      //key does already exist.
  }

  if (updateLabelValue)
    this->SetValue(templateLabel->GetValue());
}

void mitk::Label::SetDICOMCodeSequenceAsProperties(const PropertyKeyPath& basePath,
  const DICOMCodeSequence& code,
  bool withModifiers)
{
  PropertyKeyPath valuePath = basePath;
  valuePath.AddElement(LabelPropertyConstants::GetValuePropertySubName());

  PropertyKeyPath schemePath = basePath;
  schemePath.AddElement(LabelPropertyConstants::GetSchemePropertySubName());

  PropertyKeyPath meaningPath = basePath;
  meaningPath.AddElement(LabelPropertyConstants::GetMeaningPropertySubName());

  SetStringProperty(PropertyKeyPathToPropertyName(valuePath).c_str(), code.GetValue().c_str());
  SetStringProperty(PropertyKeyPathToPropertyName(schemePath).c_str(), code.GetScheme().c_str());
  SetStringProperty(PropertyKeyPathToPropertyName(meaningPath).c_str(), code.GetMeaning().c_str());

  // Handle modifiers if requested
  if (withModifiers)
  {
    const DICOMCodeSequenceWithModifiers* codeWithMods = dynamic_cast<const DICOMCodeSequenceWithModifiers*>(&code);
    if (codeWithMods != nullptr)
    {
      // Remove all existing modifiers first
      PropertyKeyPath modifierSearchPath = basePath;
      modifierSearchPath.AddAnySelection(LabelPropertyConstants::GetModifierPropertySubName());
      modifierSearchPath.AddAnyElement();

      std::string modifierPattern = PropertyKeyPathToPropertyRegEx(modifierSearchPath);
      std::regex modifierRegex(modifierPattern);

      auto propertyKeys = this->GetPropertyKeys();
      for (const auto& key : propertyKeys)
      {
        if (std::regex_match(key, modifierRegex))
        {
          this->RemoveProperty(key);
        }
      }

      // Set new modifiers
      const auto& modifiers = codeWithMods->GetModifiers();
      for (std::size_t i = 0; i < modifiers.size(); ++i)
      {
        PropertyKeyPath modifierPath = basePath;
        modifierPath.AddSelection(LabelPropertyConstants::GetModifierPropertySubName(), i);
        SetDICOMCodeSequenceAsProperties(modifierPath, modifiers[i], false);
      }
    }
  }
}

std::optional<mitk::DICOMCodeSequence> mitk::Label::GetDICOMCodeSequenceFromProperties(const PropertyKeyPath& basePath) const
{
  PropertyKeyPath valuePath = basePath;
  valuePath.AddElement(LabelPropertyConstants::GetValuePropertySubName());

  PropertyKeyPath schemePath = basePath;
  schemePath.AddElement(LabelPropertyConstants::GetSchemePropertySubName());

  PropertyKeyPath meaningPath = basePath;
  meaningPath.AddElement(LabelPropertyConstants::GetMeaningPropertySubName());

  std::string value, scheme, meaning;
  GetStringProperty(PropertyKeyPathToPropertyName(valuePath).c_str(), value);
  GetStringProperty(PropertyKeyPathToPropertyName(schemePath).c_str(), scheme);
  GetStringProperty(PropertyKeyPathToPropertyName(meaningPath).c_str(), meaning);

  auto result = DICOMCodeSequence(value, scheme, meaning);

  if (result.IsEmpty())
    return std::optional<mitk::DICOMCodeSequence>();

  return result;
}

std::optional<mitk::DICOMCodeSequenceWithModifiers> mitk::Label::GetDICOMCodeSequenceWithModifiersFromProperties(const PropertyKeyPath& basePath) const
{
  // Get the base code
  auto baseCode = GetDICOMCodeSequenceFromProperties(basePath);

  if (!baseCode.has_value())
    return std::optional<mitk::DICOMCodeSequenceWithModifiers>();

  DICOMCodeSequenceWithModifiers result(*baseCode);

  // Find all modifiers
  PropertyKeyPath modifierSearchPath = basePath;
  modifierSearchPath.AddAnySelection(LabelPropertyConstants::GetModifierPropertySubName());
  modifierSearchPath.AddElement(LabelPropertyConstants::GetValuePropertySubName());

  auto modifierEntries = FindIndexedPropertyNames(this, modifierSearchPath);

  for (const auto& entry : modifierEntries)
  {
    PropertyKeyPath modifierPath = basePath;
    modifierPath.AddSelection(LabelPropertyConstants::GetModifierPropertySubName(), entry.first);
    auto modifier = GetDICOMCodeSequenceFromProperties(modifierPath);

    // Only add non-empty modifiers
    if (modifier.has_value() && !modifier->IsEmpty())
    {
      result.AddModifier(*modifier);
    }
  }

  return result;
}

void mitk::Label::RemoveDICOMCodeSequenceProperties(const PropertyKeyPath& basePath)
{
  PropertyKeyPath valuePath = basePath;
  valuePath.AddElement(LabelPropertyConstants::GetValuePropertySubName());

  PropertyKeyPath schemePath = basePath;
  schemePath.AddElement(LabelPropertyConstants::GetSchemePropertySubName());

  PropertyKeyPath meaningPath = basePath;
  meaningPath.AddElement(LabelPropertyConstants::GetMeaningPropertySubName());

  this->RemoveProperty(PropertyKeyPathToPropertyName(valuePath).c_str());
  this->RemoveProperty(PropertyKeyPathToPropertyName(schemePath).c_str());
  this->RemoveProperty(PropertyKeyPathToPropertyName(meaningPath).c_str());

  // Also remove any modifiers
  PropertyKeyPath modifierSearchPath = basePath;
  modifierSearchPath.AddAnySelection(LabelPropertyConstants::GetModifierPropertySubName());
  modifierSearchPath.AddAnyElement();

  std::string modifierPattern = PropertyKeyPathToPropertyRegEx(modifierSearchPath);
  std::regex modifierRegex(modifierPattern);

  auto propertyKeys = this->GetPropertyKeys();
  for (const auto& key : propertyKeys)
  {
    if (std::regex_match(key, modifierRegex))
    {
      this->RemoveProperty(key);
    }
  }
}

void mitk::Label::SetAnatomicRegion(const DICOMCodeSequenceWithModifiers& code, std::size_t index)
{
  PropertyKeyPath basePath;
  basePath.AddSelection(LabelPropertyConstants::GetAnatomicRegionPropertyBaseName(), index);
  SetDICOMCodeSequenceAsProperties(basePath, code, true);
}

mitk::DICOMCodeSequenceWithModifiers mitk::Label::GetAnatomicRegion(std::size_t index) const
{
  PropertyKeyPath basePath;
  basePath.AddSelection(LabelPropertyConstants::GetAnatomicRegionPropertyBaseName(), index);
  auto result = GetDICOMCodeSequenceWithModifiersFromProperties(basePath);

  if (!result.has_value())
    mitkThrow() << "Cannot get AnatomicRegion code. Index seems to be invalid. Index: " << index;

  return *result;
}

std::size_t mitk::Label::GetAnatomicRegionCount() const
{
  PropertyKeyPath searchPath;
  searchPath.AddAnySelection(LabelPropertyConstants::GetAnatomicRegionPropertyBaseName());
  searchPath.AddElement(LabelPropertyConstants::GetValuePropertySubName());

  return FindIndexedPropertyNames(this, searchPath).size();
}

void mitk::Label::RemoveAnatomicRegion(std::size_t index)
{
  PropertyKeyPath basePath;
  basePath.AddSelection(LabelPropertyConstants::GetAnatomicRegionPropertyBaseName(), index);
  RemoveDICOMCodeSequenceProperties(basePath);
}

void mitk::Label::SetPrimaryAnatomicStructure(const DICOMCodeSequenceWithModifiers& code, std::size_t index)
{
  PropertyKeyPath basePath;
  basePath.AddSelection(LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName(), index);
  SetDICOMCodeSequenceAsProperties(basePath, code, true);
}

mitk::DICOMCodeSequenceWithModifiers mitk::Label::GetPrimaryAnatomicStructure(std::size_t index) const
{
  PropertyKeyPath basePath;
  basePath.AddSelection(LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName(), index);
  auto result = GetDICOMCodeSequenceWithModifiersFromProperties(basePath);

  if (!result.has_value())
    mitkThrow() << "Cannot get PrimaryAnatomicStructure code. Index seems to be invalid. Index: " << index;

  return *result;
}

std::size_t mitk::Label::GetPrimaryAnatomicStructureCount() const
{
  PropertyKeyPath searchPath;
  searchPath.AddAnySelection(LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName());
  searchPath.AddElement(LabelPropertyConstants::GetValuePropertySubName());

  return FindIndexedPropertyNames(this, searchPath).size();
}

void mitk::Label::RemovePrimaryAnatomicStructure(std::size_t index)
{
  PropertyKeyPath basePath;
  basePath.AddSelection(LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName(), index);
  RemoveDICOMCodeSequenceProperties(basePath);
}

void mitk::Label::SetSegmentedPropertyCategory(const DICOMCodeSequence& code)
{
  PropertyKeyPath basePath;
  basePath.AddElement(LabelPropertyConstants::GetSegmentedPropertyCategoryPropertyBaseName());
  SetDICOMCodeSequenceAsProperties(basePath, code, false);
}

std::optional<mitk::DICOMCodeSequence> mitk::Label::GetSegmentedPropertyCategory() const
{
  PropertyKeyPath basePath;
  basePath.AddElement(LabelPropertyConstants::GetSegmentedPropertyCategoryPropertyBaseName());
  return GetDICOMCodeSequenceFromProperties(basePath);
}

void mitk::Label::SetSegmentedPropertyType(const DICOMCodeSequenceWithModifiers& code)
{
  PropertyKeyPath basePath;
  basePath.AddElement(LabelPropertyConstants::GetSegmentedPropertyTypePropertyBaseName());
  SetDICOMCodeSequenceAsProperties(basePath, code, true);
}

std::optional<mitk::DICOMCodeSequenceWithModifiers> mitk::Label::GetSegmentedPropertyType() const
{
  PropertyKeyPath basePath;
  basePath.AddElement(LabelPropertyConstants::GetSegmentedPropertyTypePropertyBaseName());
  return GetDICOMCodeSequenceWithModifiersFromProperties(basePath);
}

itk::LightObject::Pointer mitk::Label::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

void mitk::Label::PrintSelf(std::ostream & /*os*/, itk::Indent /*indent*/) const
{
  // todo
}

bool mitk::Equal(const mitk::Label &leftHandSide, const mitk::Label &rightHandSide, ScalarType /*eps*/, bool verbose)
{
  MITK_INFO(verbose) << "--- Label Equal ---";

  bool returnValue = true;
  // have to be replaced until a PropertyList Equal was implemented :
  // returnValue = mitk::Equal((const mitk::PropertyList &)leftHandSide,(const mitk::PropertyList
  // &)rightHandSide,eps,verbose);

  const mitk::PropertyList::PropertyMap *lhsmap = leftHandSide.GetMap();
  const mitk::PropertyList::PropertyMap *rhsmap = rightHandSide.GetMap();

  returnValue = lhsmap->size() == rhsmap->size();

  if (!returnValue)
  {
    MITK_INFO(verbose) << "Labels in label container are not equal.";
    return returnValue;
  }

  auto lhsmapIt = lhsmap->begin();
  auto lhsmapItEnd = lhsmap->end();

  for (; lhsmapIt != lhsmapItEnd; ++lhsmapIt)
  {
    if (rhsmap->find(lhsmapIt->first) == rhsmap->end())
    {
      returnValue = false;
      break;
    }
  }

  if (!returnValue)
  {
    MITK_INFO(verbose) << "Labels in label container are not equal.";
    return returnValue;
  }

  return returnValue;
}
