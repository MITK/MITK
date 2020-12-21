/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLookupTablePropertySerializer.h"
#include "mitkStringsToNumbers.h"
#include <mitkLocaleSwitch.h>
#include <mitkLookupTableProperty.h>

#include <array>

#include <tinyxml2.h>

tinyxml2::XMLElement *mitk::LookupTablePropertySerializer::Serialize(tinyxml2::XMLDocument& doc)
{
  if (const auto *prop = dynamic_cast<const LookupTableProperty *>(m_Property.GetPointer()))
  {
    LocaleSwitch localeSwitch("C");

    LookupTable::Pointer mitkLut = const_cast<LookupTableProperty *>(prop)->GetLookupTable();
    if (mitkLut.IsNull())
      return nullptr; // really?

    vtkLookupTable *lut = mitkLut->GetVtkLookupTable();
    if (!lut)
      return nullptr;

    auto *element = doc.NewElement("LookupTable");

    double *range;
    double *rgba;

    element->SetAttribute("NumberOfColors", static_cast<int>(lut->GetNumberOfTableValues()));
    element->SetAttribute("Scale", lut->GetScale());
    element->SetAttribute("Ramp", lut->GetRamp());

    range = lut->GetHueRange();
    auto *child = doc.NewElement("HueRange");
    element->InsertEndChild(child);
    child->SetAttribute("min", boost::lexical_cast<std::string>(range[0]).c_str());
    child->SetAttribute("max", boost::lexical_cast<std::string>(range[1]).c_str());

    range = lut->GetValueRange();
    child = doc.NewElement("ValueRange");
    element->InsertEndChild(child);
    child->SetAttribute("min", boost::lexical_cast<std::string>(range[0]).c_str());
    child->SetAttribute("max", boost::lexical_cast<std::string>(range[1]).c_str());

    range = lut->GetSaturationRange();
    child = doc.NewElement("SaturationRange");
    element->InsertEndChild(child);
    child->SetAttribute("min", boost::lexical_cast<std::string>(range[0]).c_str());
    child->SetAttribute("max", boost::lexical_cast<std::string>(range[1]).c_str());

    range = lut->GetAlphaRange();
    child = doc.NewElement("AlphaRange");
    element->InsertEndChild(child);
    child->SetAttribute("min", boost::lexical_cast<std::string>(range[0]).c_str());
    child->SetAttribute("max", boost::lexical_cast<std::string>(range[1]).c_str());

    range = lut->GetTableRange();
    child = doc.NewElement("TableRange");
    element->InsertEndChild(child);
    child->SetAttribute("min", boost::lexical_cast<std::string>(range[0]).c_str());
    child->SetAttribute("max", boost::lexical_cast<std::string>(range[1]).c_str());

    child = doc.NewElement("Table");
    element->InsertEndChild(child);
    for (int index = 0; index < lut->GetNumberOfTableValues(); ++index)
    {
      auto grandChild = doc.NewElement("RgbaColor");
      rgba = lut->GetTableValue(index);
      grandChild->SetAttribute("R", boost::lexical_cast<std::string>(rgba[0]).c_str());
      grandChild->SetAttribute("G", boost::lexical_cast<std::string>(rgba[1]).c_str());
      grandChild->SetAttribute("B", boost::lexical_cast<std::string>(rgba[2]).c_str());
      grandChild->SetAttribute("A", boost::lexical_cast<std::string>(rgba[3]).c_str());
      child->InsertEndChild(grandChild);
    }
    return element;
  }
  else
    return nullptr;
}

mitk::BaseProperty::Pointer mitk::LookupTablePropertySerializer::Deserialize(const tinyxml2::XMLElement *element)
{
  if (!element)
    return nullptr;

  LocaleSwitch localeSwitch("C");

  double range[2];
  double rgba[4];

  std::array<const char*, 4> double_strings;

  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

  int numberOfColors;
  int scale;
  int ramp; // hope the int values don't change betw. vtk versions...
  if (element->QueryIntAttribute("NumberOfColors", &numberOfColors) == tinyxml2::XML_SUCCESS)
  {
    lut->SetNumberOfTableValues(numberOfColors);
  }
  else
    return nullptr;
  if (element->QueryIntAttribute("Scale", &scale) == tinyxml2::XML_SUCCESS)
  {
    lut->SetScale(scale);
  }
  else
    return nullptr;
  if (element->QueryIntAttribute("Ramp", &ramp) == tinyxml2::XML_SUCCESS)
  {
    lut->SetRamp(ramp);
  }
  else
    return nullptr;

  try
  {
    auto *child = element->FirstChildElement("HueRange");
    if (child)
    {
      double_strings[0] = child->Attribute("min");
      double_strings[1] = child->Attribute("max");

      if (nullptr == double_strings[0] || nullptr == double_strings[1])
        return nullptr;

      StringsToNumbers<double>(2, double_strings, range);
      lut->SetHueRange(range);
    }

    child = element->FirstChildElement("ValueRange");
    if (child)
    {
      double_strings[0] = child->Attribute("min");
      double_strings[1] = child->Attribute("max");

      if (nullptr == double_strings[0] || nullptr == double_strings[1])
        return nullptr;

      StringsToNumbers<double>(2, double_strings, range);
      lut->SetValueRange(range);
    }

    child = element->FirstChildElement("SaturationRange");
    if (child)
    {
      double_strings[0] = child->Attribute("min");
      double_strings[1] = child->Attribute("max");

      if (nullptr == double_strings[0] || nullptr == double_strings[1])
        return nullptr;

      StringsToNumbers<double>(2, double_strings, range);
      lut->SetSaturationRange(range);
    }

    child = element->FirstChildElement("AlphaRange");
    if (child)
    {
      double_strings[0] = child->Attribute("min");
      double_strings[1] = child->Attribute("max");

      if (nullptr == double_strings[0] || nullptr == double_strings[1])
        return nullptr;

      StringsToNumbers<double>(2, double_strings, range);
      lut->SetAlphaRange(range);
    }

    child = element->FirstChildElement("TableRange");
    if (child)
    {
      double_strings[0] = child->Attribute("min");
      double_strings[1] = child->Attribute("max");

      if (nullptr == double_strings[0] || nullptr == double_strings[1])
        return nullptr;

      StringsToNumbers<double>(2, double_strings, range);
      lut->SetTableRange(range);
    }

    child = element->FirstChildElement("Table");
    if (child)
    {
      unsigned int index(0);
      for (auto *grandChild = child->FirstChildElement("RgbaColor"); grandChild;
           grandChild = grandChild->NextSiblingElement("RgbaColor"))
      {
        double_strings[0] = grandChild->Attribute("R");
        double_strings[1] = grandChild->Attribute("G");
        double_strings[2] = grandChild->Attribute("B");
        double_strings[3] = grandChild->Attribute("A");

        if (nullptr == double_strings[0] || nullptr == double_strings[1] || nullptr == double_strings[2] || nullptr == double_strings[3])
          return nullptr;

        StringsToNumbers<double>(4, double_strings, rgba);
        lut->SetTableValue(index, rgba);
        ++index;
      }
    }
  }
  catch (boost::bad_lexical_cast &e)
  {
    MITK_ERROR << "Could not parse string as number: " << e.what();
    return nullptr;
  }

  LookupTable::Pointer mitkLut = LookupTable::New();
  mitkLut->SetVtkLookupTable(lut);

  return LookupTableProperty::New(mitkLut).GetPointer();
}
