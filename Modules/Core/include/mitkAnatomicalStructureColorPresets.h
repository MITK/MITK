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

#ifndef ANATOMICALSTRUCTURECOLORPRESETS_H_HEADER
#define ANATOMICALSTRUCTURECOLORPRESETS_H_HEADER

#include <vtkXMLParser.h>
#include <MitkCoreExports.h>
#include <mitkColorProperty.h>
#include <map>
#include <string>

namespace mitk {

class MITKCORE_EXPORT AnatomicalStructureColorPresets : public vtkXMLParser
{
public:
  struct Category
  {
    std::string codeValue;
    std::string codeScheme;
    std::string codeName;
    Category() = default;
    Category(std::string value, std::string scheme, std::string name) : codeValue(value), codeScheme(scheme), codeName(name){}
  };

  struct Type
  {
    struct Modifier
    {
      std::string codeValue;
      std::string codeScheme;
      std::string codeName;
      Modifier() = default;
      Modifier(std::string value, std::string scheme, std::string name) : codeValue(value), codeScheme(scheme), codeName(name){}
    };
    std::string codeValue;
    std::string codeScheme;
    std::string codeName;
    Modifier modifier;
    Type() = default;
    Type(std::string value, std::string scheme, std::string name) : codeValue(value), codeScheme(scheme), codeName(name){}
  };

  static AnatomicalStructureColorPresets *New();
  vtkTypeMacro(AnatomicalStructureColorPresets,vtkXMLParser);

  bool LoadPreset();
  bool LoadPreset(const std::string& fileName);
  Category GetCategory(const std::string& name);
  Type GetType(const std::string& name);
  Color GetColor(const std::string& name);
  std::map<std::string, Category> const GetCategoryPresets();
  std::map<std::string, Type> const GetTypePresets();
  std::map<std::string, Color> const GetColorPresets();
  void NewPresets(std::map<std::string, Category>& newCategory, std::map<std::string, Type>& newType, std::map<std::string, Color>& newColor);

protected:
  AnatomicalStructureColorPresets() = default;
  ~AnatomicalStructureColorPresets() override = default;

private:
  //##Documentation
  //## @brief method used in XLM-Reading; gets called when a start-tag is read
  void StartElement (const char *elementName, const char **atts) override;

  void Save();

  //##Documentation
  //## @brief reads an XML-String-Attribute
  std::string ReadXMLStringAttribute(const std::string& name, const char **atts);

  static const std::string PRESET;
  static const std::string CATEGORY;
  static const std::string TYPE;
  static const std::string MODIFIER;
  static const std::string COLOR;
  static const std::string CODE_VALUE;
  static const std::string CODE_SCHEME;
  static const std::string CODE_NAME;

  static const std::string COLOR_R;
  static const std::string COLOR_G;
  static const std::string COLOR_B;

  std::string m_presetName;
  std::map<std::string, Category> m_Category;
  std::map<std::string, Type> m_Type;
  std::map<std::string, Color> m_Color;
  std::string m_XmlFileName;
};
}
#endif
