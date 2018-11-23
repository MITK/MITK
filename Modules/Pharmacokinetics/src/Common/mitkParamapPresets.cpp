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

#include "mitkParamapPresets.h"
#include <vtkObjectFactory.h>

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace mitk {

  const std::string ParamapPresets::PRESET = "preset";
  const std::string ParamapPresets::TYPE = "type";
  const std::string ParamapPresets::CODE_VALUE = "code_value";
  const std::string ParamapPresets::CODE_SCHEME = "coding_scheme";

  vtkStandardNewMacro(ParamapPresets);

  bool ParamapPresets::LoadPreset()
  {
    us::ModuleResource presetResource = us::GetModuleContext()->GetModule()->GetResource("mitkParamapPresets.xml");
    if (!presetResource) return false;

    us::ModuleResourceStream presetStream(presetResource);
    vtkXMLParser::SetStream(&presetStream);
    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO<<"ParamapPresets::LoadPreset xml file cannot parse!"<<std::endl;
#endif
      return false;
    }
    return true;
  }

  bool ParamapPresets::LoadPreset(const std::string& fileName)
  {
    if (fileName.empty())
      return false;

    vtkXMLParser::SetFileName(fileName.c_str());

    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO<<"ParamapPresets::LoadPreset xml file cannot parse!"<<std::endl;
#endif
    }

    return true;

  }

  void  ParamapPresets::StartElement(const char *elementName, const char **atts)
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

  std::string ParamapPresets::ReadXMLStringAttribute(const std::string& name, const char **atts)
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


  ParamapPresets::Type ParamapPresets::GetType(const std::string& name)
  {
    return m_Type[name];
  }



  std::map<std::string, ParamapPresets::Type> const ParamapPresets::GetTypePresets()
  {
    return m_Type;
  }

 

  void ParamapPresets::Save()
  {
    //Not yet implemented
  }

  void ParamapPresets::NewPresets(std::map<std::string, ParamapPresets::Type>& newType)
  {
   m_Type = newType;
    this->Save();
  }

}
