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

#ifndef LEVELWINDOWPRESET_H_HEADER
#define LEVELWINDOWPRESET_H_HEADER

#include <vtkXMLParser.h>
#include <mitkXMLWriter.h>
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

  void saveXML(mitk::XMLWriter& xmlWriter);
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
