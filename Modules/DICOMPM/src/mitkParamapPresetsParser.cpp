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

#include "mitkParamapPresetsParser.h"
#include <vtkObjectFactory.h>

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace mitk {

  const std::string ParamapPresetsParser::PRESET = "preset";
  const std::string ParamapPresetsParser::TYPE = "type";
  const std::string ParamapPresetsParser::CODE_VALUE = "code_value";
  const std::string ParamapPresetsParser::CODE_SCHEME = "coding_scheme";

  vtkStandardNewMacro(ParamapPresetsParser);

  bool ParamapPresetsParser::LoadPreset()
  {
    us::ModuleResource presetResource = us::GetModuleContext()->GetModule()->GetResource("mitkParamapPresets.xml");
    if (!presetResource) return false;

    us::ModuleResourceStream presetStream(presetResource);
    vtkXMLParser::SetStream(&presetStream);
    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO<<"ParamapPresetsParser::LoadPreset xml file cannot parse!"<<std::endl;
#endif
      return false;
    }
    return true;
  }

  bool ParamapPresetsParser::LoadPreset(const std::string& fileName)
  {
    if (fileName.empty())
      return false;

    vtkXMLParser::SetFileName(fileName.c_str());

    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO<<"ParamapPresetsParser::LoadPreset xml file cannot parse!"<<std::endl;
#endif
    }

    return true;

  }

  void  ParamapPresetsParser::StartElement(const char *elementName, const char **atts)
  {
    std::string elementNameString = elementName;
    if (elementNameString == PRESET)
    {
      m_presetName = ReadXMLStringAttribute("name", atts);
    }

    else if (elementNameString == TYPE)
    {
      std::string value = ReadXMLStringAttribute(CODE_VALUE, atts);
      std::string scheme = ReadXMLStringAttribute(CODE_SCHEME, atts);
      m_Type[m_presetName] = Type(value, scheme);
    }

  }

  std::string ParamapPresetsParser::ReadXMLStringAttribute(const std::string& name, const char **atts)
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


  ParamapPresetsParser::Type ParamapPresetsParser::GetType(const std::string& name)
  {
    return m_Type[name];
  }



  std::map<std::string, ParamapPresetsParser::Type> const ParamapPresetsParser::GetTypePresets()
  {
    return m_Type;
  }

 



  void ParamapPresetsParser::NewPresets(std::map<std::string, ParamapPresetsParser::Type>& newType)
  {
   m_Type = newType;
  }

}
