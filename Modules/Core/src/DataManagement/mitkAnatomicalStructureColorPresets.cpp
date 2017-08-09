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

#include "mitkAnatomicalStructureColorPresets.h"
#include <vtkObjectFactory.h>

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace mitk {

  const std::string AnatomicalStructureColorPresets::PRESET = "preset";
  const std::string AnatomicalStructureColorPresets::CATEGORY = "category";
  const std::string AnatomicalStructureColorPresets::TYPE = "type";
  const std::string AnatomicalStructureColorPresets::MODIFIER = "modifier";
  const std::string AnatomicalStructureColorPresets::COLOR = "color";

  const std::string AnatomicalStructureColorPresets::CODE_VALUE = "code_value";
  const std::string AnatomicalStructureColorPresets::CODE_SCHEME = "coding_scheme";
  const std::string AnatomicalStructureColorPresets::CODE_NAME = "code_name";

  const std::string AnatomicalStructureColorPresets::COLOR_R = "rgb_r";
  const std::string AnatomicalStructureColorPresets::COLOR_G = "rgb_g";
  const std::string AnatomicalStructureColorPresets::COLOR_B = "rgb_b";


  vtkStandardNewMacro(AnatomicalStructureColorPresets);

  bool AnatomicalStructureColorPresets::LoadPreset()
  {
    us::ModuleResource presetResource = us::GetModuleContext()->GetModule()->GetResource("mitkAnatomicalStructureColorPresets.xml");
    if (!presetResource) return false;

    us::ModuleResourceStream presetStream(presetResource);
    vtkXMLParser::SetStream(&presetStream);
    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO<<"AnatomicalStructureColorPresets::LoadPreset xml file cannot parse!"<<std::endl;
#endif
      return false;
    }
    return true;
  }

  bool AnatomicalStructureColorPresets::LoadPreset(const std::string& fileName)
  {
    if (fileName.empty())
      return false;

    vtkXMLParser::SetFileName(fileName.c_str());

    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO<<"AnatomicalStructureColorPresets::LoadPreset xml file cannot parse!"<<std::endl;
#endif
    }

    return true;

  }

  void  AnatomicalStructureColorPresets::StartElement(const char *elementName, const char **atts)
  {
    std::string elementNameString = elementName;
    if (elementNameString == PRESET)
    {
      m_presetName = ReadXMLStringAttribute("name", atts);
    }
    else if (elementNameString == CATEGORY)
    {
      std::string value = ReadXMLStringAttribute(CODE_VALUE, atts);
      std::string scheme = ReadXMLStringAttribute(CODE_SCHEME, atts);
      std::string name = ReadXMLStringAttribute(CODE_NAME, atts);
      m_Category[m_presetName] = Category(value, scheme, name);
    }
    else if (elementNameString == TYPE)
    {
      std::string value = ReadXMLStringAttribute(CODE_VALUE, atts);
      std::string scheme = ReadXMLStringAttribute(CODE_SCHEME, atts);
      std::string name = ReadXMLStringAttribute(CODE_NAME, atts);
      m_Type[m_presetName] = Type(value, scheme, name);
    }
    else if (elementNameString == MODIFIER)
    {
      std::string value = ReadXMLStringAttribute(CODE_VALUE, atts);
      std::string scheme = ReadXMLStringAttribute(CODE_SCHEME, atts);
      std::string name = ReadXMLStringAttribute(CODE_NAME, atts);
      m_Type[m_presetName].modifier = Type::Modifier(value, scheme, name);
    }
    else if (elementNameString == COLOR)
    {
      std::string red = ReadXMLStringAttribute(COLOR_R, atts);
      float r = atof(red.c_str());
      std::string green = ReadXMLStringAttribute(COLOR_G, atts);
      float g = atof(green.c_str());
      std::string blue = ReadXMLStringAttribute(COLOR_B, atts);
      float b = atof(blue.c_str());
      Color color;
      color.SetRed(r); color.SetGreen(g); color.SetBlue(b);
      m_Color[m_presetName] = color;
    }
  }

  std::string AnatomicalStructureColorPresets::ReadXMLStringAttribute(const std::string& name, const char **atts)
  {
    if (atts)
    {
      const char** attsIter = atts;

      while (*attsIter != nullptr)
      {
        if (name == *attsIter)
        {
          return *(++attsIter);
        }
        attsIter += 2;
      }
    }

    return std::string();
  }

  AnatomicalStructureColorPresets::Category AnatomicalStructureColorPresets::GetCategory(const std::string& name)
  {
    return m_Category[name];
  }

  AnatomicalStructureColorPresets::Type AnatomicalStructureColorPresets::GetType(const std::string& name)
  {
    return m_Type[name];
  }

  Color AnatomicalStructureColorPresets::GetColor(const std::string& name)
  {
    return m_Color[name];
  }

  std::map<std::string, AnatomicalStructureColorPresets::Category> const AnatomicalStructureColorPresets::GetCategoryPresets()
  {
    return m_Category;
  }

  std::map<std::string, AnatomicalStructureColorPresets::Type> const AnatomicalStructureColorPresets::GetTypePresets()
  {
    return m_Type;
  }

  std::map<std::string, Color> const AnatomicalStructureColorPresets::GetColorPresets()
  {
    return m_Color;
  }

  void AnatomicalStructureColorPresets::Save()
  {
    //Not yet implemented
  }

  void AnatomicalStructureColorPresets::NewPresets(std::map<std::string, AnatomicalStructureColorPresets::Category>& newCategory, std::map<std::string, AnatomicalStructureColorPresets::Type>& newType, std::map<std::string, Color>& newColor)
  {
    m_Category = newCategory;
    m_Type = newType;
    m_Color = newColor;
    this->Save();
  }

}
