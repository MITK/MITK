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

#include "mitkLevelWindowPreset.h"
#include <vtkObjectFactory.h>

#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace mitk
{
  const std::string LevelWindowPreset::PRESET = "preset";

  vtkStandardNewMacro(LevelWindowPreset);

  LevelWindowPreset::LevelWindowPreset() {}
  LevelWindowPreset::~LevelWindowPreset() {}
  bool LevelWindowPreset::LoadPreset()
  {
    us::ModuleResource presetResource = us::GetModuleContext()->GetModule()->GetResource("mitkLevelWindowPresets.xml");
    if (!presetResource)
      return false;

    us::ModuleResourceStream presetStream(presetResource);
    vtkXMLParser::SetStream(&presetStream);
    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO << "LevelWindowPreset::LoadPreset xml file cannot parse!" << std::endl;
#endif
      return false;
    }
    return true;
  }

  bool LevelWindowPreset::LoadPreset(std::string fileName)
  {
    if (fileName.empty())
      return false;

    vtkXMLParser::SetFileName(fileName.c_str());

    if (!vtkXMLParser::Parse())
    {
#ifdef INTERDEBUG
      MITK_INFO << "LevelWindowPreset::LoadPreset xml file cannot parse!" << std::endl;
#endif
    }

    return true;
  }

  void LevelWindowPreset::StartElement(const char *elementName, const char **atts)
  {
    std::string elementNameString = elementName;
    if (elementNameString == PRESET)
    {
      std::string name = ReadXMLStringAttribut("NAME", atts);
      std::string level = ReadXMLStringAttribut("LEVEL", atts);
      double lev = atof(level.c_str());
      std::string window = ReadXMLStringAttribut("WINDOW", atts);
      double win = atof(window.c_str());
      m_Level[name] = lev;
      m_Window[name] = win;
    }
  }

  std::string LevelWindowPreset::ReadXMLStringAttribut(std::string name, const char **atts)
  {
    if (atts)
    {
      const char **attsIter = atts;

      while (*attsIter)
      {
        if (name == *attsIter)
        {
          attsIter++;
          return *attsIter;
        }
        attsIter++;
        attsIter++;
      }
    }

    return std::string();
  }

  double LevelWindowPreset::getLevel(std::string name) { return m_Level[name]; }
  double LevelWindowPreset::getWindow(std::string name) { return m_Window[name]; }
  std::map<std::string, double> &LevelWindowPreset::getLevelPresets() { return m_Level; }
  std::map<std::string, double> &LevelWindowPreset::getWindowPresets() { return m_Window; }
  void LevelWindowPreset::save()
  {
    // XMLWriter writer(m_XmlFileName.c_str());
    // saveXML(writer);
  }

  void LevelWindowPreset::newPresets(std::map<std::string, double> newLevel, std::map<std::string, double> newWindow)
  {
    m_Level = newLevel;
    m_Window = newWindow;
    save();
  }
}
