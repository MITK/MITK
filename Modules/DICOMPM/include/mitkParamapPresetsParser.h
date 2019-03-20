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

#ifndef PARAMAPRPRESETS_H_HEADER
#define PARAMAPPRESETS_H_HEADER

#include <MitkDICOMPMExports.h>
#include <vtkXMLParser.h>
#include <map>
#include <string>
#include <vtkSetGet.h>



namespace mitk {


class MITKDICOMPM_EXPORT ParamapPresetsParser : public vtkXMLParser
{
public:
  struct Type
  {
     std::string codeValue;
     std::string codeScheme;
     Type() = default;
     Type(std::string value, std::string scheme) : codeValue(value), codeScheme(scheme){}
   };

  using ParamapPrestsType = std::map<std::string, Type>;
  static ParamapPresetsParser *New();
  vtkTypeMacro(ParamapPresetsParser,vtkXMLParser);



  bool LoadPreset();
  bool LoadPreset(const std::string& fileName);
  Type GetType(const std::string& name);
  ParamapPrestsType  const GetTypePresets();
  void NewPresets(ParamapPrestsType & newType);


protected:
  ParamapPresetsParser() = default;
  ~ParamapPresetsParser() override = default;

private:
  //##Documentation
  //## @brief method used in XLM-Reading; gets called when a start-tag is read
  void StartElement (const char *elementName, const char **atts) override;


  //##Documentation
  //## @brief reads an XML-String-Attribute
  std::string ReadXMLStringAttribute(const std::string& name, const char **atts);

  static const std::string PRESET;
  static const std::string TYPE;
  static const std::string CODE_VALUE;
  static const std::string CODE_SCHEME;

  std::string m_presetName;
  ParamapPrestsType  m_Type;
  std::string m_XmlFileName;
};
}
#endif
