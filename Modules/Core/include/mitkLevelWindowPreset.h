/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLevelWindowPreset_h
#define mitkLevelWindowPreset_h

#include <MitkCoreExports.h>
#include <map>
#include <string>
#include <vtkXMLParser.h>

namespace mitk
{
  class MITKCORE_EXPORT LevelWindowPreset : public vtkXMLParser
  {
  public:
    static LevelWindowPreset *New();
    vtkTypeMacro(LevelWindowPreset, vtkXMLParser);

    bool LoadPreset();
    bool LoadPreset(std::string fileName);
    double getLevel(std::string name);
    double getWindow(std::string window);
    std::map<std::string, double> &getLevelPresets();
    std::map<std::string, double> &getWindowPresets();
    void newPresets(std::map<std::string, double> newLevel, std::map<std::string, double> newWindow);

  protected:
    LevelWindowPreset();
    ~LevelWindowPreset() override;

  private:
    //##Documentation
    //## @brief method used in XLM-Reading; gets called when a start-tag is read
    void StartElement(const char *elementName, const char **atts) override;

    // void saveXML(mitk::XMLWriter& xmlWriter);
    void save();

    //##Documentation
    //## @brief reads an XML-String-Attribute
    std::string ReadXMLStringAttribut(std::string name, const char **atts);

    static const std::string PRESET;
    std::map<std::string, double> m_Level;
    std::map<std::string, double> m_Window;
    std::string m_XmlFileName;
  };
}
#endif
