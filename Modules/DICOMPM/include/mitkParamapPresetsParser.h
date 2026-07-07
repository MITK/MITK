/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkParamapPresetsParser_h
#define mitkParamapPresetsParser_h

#include <MitkDICOMPMExports.h>
#include <vtkXMLParser.h>
#include <map>
#include <string>
#include <vtkSetGet.h>



namespace mitk {

/**
 * \brief Parser for DICOM Parametric Map (PM) preset definitions stored in XML format.
 *
 * This class reads XML files that define parametric map type presets, each consisting of
 * a coded value and a coding scheme designator. These presets are used to populate
 * DICOM parametric map objects with standardized coded entries.
 *
 * The default preset file is loaded from the module resources (\c mitkParamapPresets.xml).
 * Alternatively, a custom XML file can be specified.
 *
 * \sa vtkXMLParser
 */
class MITKDICOMPM_EXPORT ParamapPresetsParser : public vtkXMLParser
{
public:
  /**
   * \brief Represents a single parametric map type preset with a coded value and coding scheme.
   */
  struct Type
  {
     std::string codeValue;   ///< \brief Coded value identifying the parametric map type.
     std::string codeScheme;  ///< \brief Coding scheme designator (e.g., "DCM").
     Type() = default;

     /**
      * \brief Construct a Type with a given coded value and coding scheme.
      *
      * \param[in] value  The coded value string.
      * \param[in] scheme The coding scheme designator string.
      */
     Type(std::string value, std::string scheme) : codeValue(value), codeScheme(scheme){}
   };

  /** \brief Map type associating preset names to their Type definitions. */
  using ParamapPrestsType = std::map<std::string, Type>;

  /** \brief VTK-style factory method. */
  static ParamapPresetsParser *New();
  vtkTypeMacro(ParamapPresetsParser,vtkXMLParser);

  /**
   * \brief Load presets from the default module resource file.
   *
   * Loads and parses the built-in \c mitkParamapPresets.xml from the module resources.
   *
   * \return \c true if the presets were loaded and parsed successfully, \c false otherwise.
   */
  bool LoadPreset();

  /**
   * \brief Load presets from a custom XML file.
   *
   * \param[in] fileName Path to the XML file containing preset definitions.
   * \return \c true if the file was parsed successfully, \c false if the file name is
   *         empty or parsing failed.
   */
  bool LoadPreset(const std::string& fileName);

  /**
   * \brief Retrieve the type definition for a given preset name.
   *
   * \param[in] name The name of the preset to look up.
   * \return The Type associated with \p name. Returns a default-constructed Type if
   *         the name is not found.
   */
  Type GetType(const std::string& name);

  /**
   * \brief Get all currently loaded type presets.
   *
   * \return A copy of the map containing all preset name-to-Type associations.
   */
  ParamapPrestsType  const GetTypePresets();

  /**
   * \brief Replace all current presets with a new set.
   *
   * \param[in] newType The new map of preset name-to-Type associations to use.
   */
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
