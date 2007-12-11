/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkLevelWindowPreset.h"
#include "mitkStandardFileLocations.h"

namespace mitk {

  const std::string LevelWindowPreset::PRESET = "preset";

LevelWindowPreset::LevelWindowPreset()
{
}

LevelWindowPreset::~LevelWindowPreset()
{
}

bool LevelWindowPreset::LoadPreset()
{
  m_XmlFileName = mitk::StandardFileLocations::GetInstance()->FindFile("mitkLevelWindowPresets.xml", "Config");

  if (!m_XmlFileName.empty()) 
    return LoadPreset(m_XmlFileName);
  else
    return false;
}

bool LevelWindowPreset::LoadPreset(std::string fileName)
{
  if ( fileName.empty() )
    return false;

  vtkXMLParser::SetFileName( fileName.c_str() );

  if ( !vtkXMLParser::Parse() )
  {
#ifdef INTERDEBUG
    std::cout<<"LevelWindowPreset::LoadPreset xml file cannot parse!"<<std::endl;
#endif
  }

  return true;

}

void  LevelWindowPreset::StartElement (const char *elementName, const char **atts)
{
  if ( elementName == PRESET )
  {
    std::string name = ReadXMLStringAttribut( "NAME", atts );
    std::string level = ReadXMLStringAttribut( "LEVEL", atts );
    double lev = atof(level.c_str());
    std::string window = ReadXMLStringAttribut( "WINDOW", atts );
    double win = atof(window.c_str());
    m_Level[name] = lev;
    m_Window[name] = win;
  }
}

std::string LevelWindowPreset::ReadXMLStringAttribut( std::string name, const char** atts )
{
  if(atts)
  {
    const char** attsIter = atts;

    while(*attsIter)
    {
      if ( name == *attsIter )
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

double LevelWindowPreset::getLevel(std::string name)
{
  return m_Level[name];
}

double LevelWindowPreset::getWindow(std::string name)
{
  return m_Window[name];
}

std::map<std::string, double>& LevelWindowPreset::getLevelPresets()
{
  return m_Level;
}

std::map<std::string, double>& LevelWindowPreset::getWindowPresets()
{
  return m_Window;
}

void LevelWindowPreset::save()
{
  XMLWriter writer(m_XmlFileName.c_str());
  saveXML(writer);
}

void LevelWindowPreset::saveXML(mitk::XMLWriter& xmlWriter)
{
  xmlWriter.BeginNode("mitkLevelWindowPresets");
  for( std::map<std::string, double>::iterator iter = m_Level.begin(); iter != m_Level.end(); iter++ ) {
    std::string item = ((*iter).first.c_str());
    xmlWriter.BeginNode("preset");
    xmlWriter.WriteProperty("NAME", item);
    double levItem = m_Level[item];
    xmlWriter.WriteProperty("LEVEL", levItem);
    double winItem = m_Window[item];
    xmlWriter.WriteProperty("WINDOW", winItem);
    xmlWriter.EndNode();
  }
  xmlWriter.EndNode();
}

void LevelWindowPreset::newPresets(std::map<std::string, double> newLevel, std::map<std::string, double> newWindow)
{
  m_Level = newLevel;
  m_Window = newWindow;
  save();
}
}
