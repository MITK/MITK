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

const mitk::Label::PixelType mitk::Label::MAX_LABEL_VALUE = std::numeric_limits<mitk::Label::PixelType>::max();

namespace
{
  const std::map<std::string, std::string>& GetPropertyNameLookup()
  {
    static const std::map<std::string, std::string> lookup = {
      // Mapping for legacy label properties that were wrong and therefore deprecated
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::ANATOMIC_REGION_CODE_MEANING_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::ANATOMIC_REGION_CODE_SCHEME_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::ANATOMIC_REGION_CODE_VALUE_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_SUB_PATH()),
        "PatientName" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0062, 0x0002) + mitk::DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_SUB_PATH()),
        "PatientName" },
        // Mapping between DICOMTag paths and human readable keywords used internally as for label properties
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_LABEL_SUB_PATH()), "name" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_DESCRIPTION_SUB_PATH()), "description" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_SUB_PATH()), "algorithm_type" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_ALGORITHM_NAME_SUB_PATH()), "algorithm_name" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_TRACKING_ID_SUB_PATH()), "tracking_id" },
        { mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_TRACKING_UID_SUB_PATH()), "tracking_uid" }
    };
    return lookup;
  }

  std::string EnsureInternalPropertyName(const std::string& externalName)
  {
    auto mapping = GetPropertyNameLookup();
    auto finding = mapping.find(externalName);

    if (finding != mapping.end())
    {
      return finding->second;
    }

    return externalName;
  }
}

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

mitk::Label::Label(const Label &other) : PropertyList(other)
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

  auto internalKey = EnsureInternalPropertyName(propertyKey);
  Superclass::SetProperty(internalKey, property, contextName, fallBackOnDefaultContext);
}

mitk::BaseProperty::ConstPointer mitk::Label::GetConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext) const
{
  auto internalKey = EnsureInternalPropertyName(propertyKey);
  return Superclass::GetConstProperty(internalKey, contextName, fallBackOnDefaultContext);
}

mitk::BaseProperty* mitk::Label::GetNonConstProperty(const std::string& propertyKey, const std::string& contextName, bool fallBackOnDefaultContext)
{
  auto internalKey = EnsureInternalPropertyName(propertyKey);
  return Superclass::GetNonConstProperty(internalKey, contextName, fallBackOnDefaultContext);
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
  std::string trackingID = std::to_string(this->GetValue());
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
  std::string trackingUID = std::to_string(this->GetValue());
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
  if (algoType == AlgorithmType::Undefined)
  {
    this->RemoveProperty("algorithm_type");
  }
  else
  {
    std::string text = "AUTOMATIC";
    if (algoType == AlgorithmType::MANUAL)
      text = "MANUAL";
    else if (algoType == AlgorithmType::SEMIAUTOMATIC)
      text = "SEMIAUTOMATIC";

    mitk::StringProperty* property = dynamic_cast<mitk::StringProperty*>(this->GetProperty("algorithm_type"));
    if (property != nullptr)
      // Update Property
      property->SetValue(text);
    else
      // Create new Property
      SetStringProperty("algorithm_type", text.c_str());
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
