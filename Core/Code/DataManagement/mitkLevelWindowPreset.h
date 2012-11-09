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

#ifndef LEVELWINDOWPRESET_H_HEADER
#define LEVELWINDOWPRESET_H_HEADER

#include <vtkXMLParser.h>
#include <MitkExports.h>
#include <map>
#include <string>

namespace mitk {

class MITK_CORE_EXPORT LevelWindowPreset : public vtkXMLParser
{
public:
  static LevelWindowPreset *New();
  vtkTypeMacro(LevelWindowPreset,vtkXMLParser);

  bool LoadPreset();
  bool LoadPreset(std::string fileName);
  double getLevel(std::string name);
  double getWindow(std::string window);
  std::map<std::string, double>& getLevelPresets();
  std::map<std::string, double>& getWindowPresets();
  void newPresets(std::map<std::string, double> newLevel, std::map<std::string, double> newWindow);
protected:
  LevelWindowPreset();
  ~LevelWindowPreset();

private:
  //##Documentation
  //## @brief method used in XLM-Reading; gets called when a start-tag is read
  void StartElement (const char *elementName, const char **atts);

  //void saveXML(mitk::XMLWriter& xmlWriter);
  void save();

  //##Documentation
  //## @brief reads an XML-String-Attribute
  std::string ReadXMLStringAttribut( std::string name, const char** atts);

  static const std::string PRESET;
  std::map<std::string, double> m_Level;
  std::map<std::string, double> m_Window;
  std::string m_XmlFileName;
};
}
#endif
