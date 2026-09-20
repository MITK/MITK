/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBasePropertySerializer.h>
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkLocaleSwitch.h>
#include <mitkStringsToNumbers.h>

#include <array>

#include <tinyxml2.h>

namespace
{
  tinyxml2::XMLElement *SerializeLevel(tinyxml2::XMLDocument &doc, const mitk::IsoDoseLevel &level)
  {
    auto *element = doc.NewElement("level");
    const auto color = level.GetColor();

    element->SetAttribute("dose", mitk::ToString(level.GetDoseValue()).c_str());
    element->SetAttribute("r", mitk::ToString(color.GetRed()).c_str());
    element->SetAttribute("g", mitk::ToString(color.GetGreen()).c_str());
    element->SetAttribute("b", mitk::ToString(color.GetBlue()).c_str());
    element->SetAttribute("isoLine", level.GetVisibleIsoLine());
    element->SetAttribute("colorWash", level.GetVisibleColorWash());

    return element;
  }

  mitk::IsoDoseLevel::Pointer DeserializeLevel(const tinyxml2::XMLElement *element)
  {
    const std::array<const char *, 4> strings = {
      element->Attribute("dose"), element->Attribute("r"), element->Attribute("g"), element->Attribute("b")};

    for (const auto *string : strings)
    {
      if (nullptr == string)
      {
        MITK_ERROR << "Iso dose level element lacks a dose or color attribute";
        return nullptr;
      }
    }

    std::array<double, 4> values{};

    try
    {
      mitk::StringsToNumbers<double>(4, strings, values);
    }
    catch (const mitk::BadLexicalCast &e)
    {
      MITK_ERROR << "Could not parse iso dose level attribute as number: " << e.what();
      return nullptr;
    }

    bool isoLine = false;
    bool colorWash = false;

    if (tinyxml2::XML_SUCCESS != element->QueryBoolAttribute("isoLine", &isoLine) ||
        tinyxml2::XML_SUCCESS != element->QueryBoolAttribute("colorWash", &colorWash))
    {
      MITK_ERROR << "Iso dose level element lacks a valid visibility attribute";
      return nullptr;
    }

    mitk::IsoDoseLevel::ColorType color;
    color.SetRed(static_cast<float>(values[1]));
    color.SetGreen(static_cast<float>(values[2]));
    color.SetBlue(static_cast<float>(values[3]));

    return mitk::IsoDoseLevel::New(values[0], color, isoLine, colorWash);
  }
}

namespace mitk
{
  class IsoDoseLevelSetPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(IsoDoseLevelSetPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      const auto *prop = dynamic_cast<const IsoDoseLevelSetProperty *>(m_Property.GetPointer());

      if (nullptr == prop)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      auto *element = doc.NewElement("isoDoseLevelSet");
      const auto *levelSet = prop->GetValue();

      if (nullptr != levelSet)
      {
        for (auto iter = levelSet->Begin(); iter != levelSet->End(); ++iter)
          element->InsertEndChild(SerializeLevel(doc, *iter));
      }

      return element;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (nullptr == element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      auto levelSet = IsoDoseLevelSet::New();

      for (const auto *levelElement = element->FirstChildElement("level"); nullptr != levelElement;
           levelElement = levelElement->NextSiblingElement("level"))
      {
        const auto level = DeserializeLevel(levelElement);

        if (level.IsNull())
          return nullptr;

        levelSet->SetIsoDoseLevel(level);
      }

      return IsoDoseLevelSetProperty::New(levelSet).GetPointer();
    }

  protected:
    IsoDoseLevelSetPropertySerializer() = default;
    ~IsoDoseLevelSetPropertySerializer() override = default;
  };

  class IsoDoseLevelVectorPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(IsoDoseLevelVectorPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      const auto *prop = dynamic_cast<const IsoDoseLevelVectorProperty *>(m_Property.GetPointer());

      if (nullptr == prop)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      auto *element = doc.NewElement("isoDoseLevelVector");
      const auto *levelVector = prop->GetValue();

      if (nullptr != levelVector)
      {
        for (const auto &level : *levelVector)
        {
          if (level.IsNotNull())
            element->InsertEndChild(SerializeLevel(doc, *level));
        }
      }

      return element;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (nullptr == element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      auto levelVector = IsoDoseLevelVector::New();

      for (const auto *levelElement = element->FirstChildElement("level"); nullptr != levelElement;
           levelElement = levelElement->NextSiblingElement("level"))
      {
        const auto level = DeserializeLevel(levelElement);

        if (level.IsNull())
          return nullptr;

        levelVector->push_back(level);
      }

      return IsoDoseLevelVectorProperty::New(levelVector).GetPointer();
    }

  protected:
    IsoDoseLevelVectorPropertySerializer() = default;
    ~IsoDoseLevelVectorPropertySerializer() override = default;
  };
}

MITK_REGISTER_SERIALIZER(IsoDoseLevelSetPropertySerializer);
MITK_REGISTER_SERIALIZER(IsoDoseLevelVectorPropertySerializer);
