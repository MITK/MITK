/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAnatomicalStructureColorPresets_h
#define mitkAnatomicalStructureColorPresets_h

#include <vtkXMLParser.h>
#include <MitkCoreExports.h>
#include <mitkColorProperty.h>
#include <map>
#include <string>

namespace mitk {

/** \brief Provides anatomical structure color presets loaded from XML.
 *
 * Parses an XML file containing color, category, and type information for anatomical
 * structures as defined by DICOM coding schemes. Used to associate structures with
 * standardized colors, categories, and types.
 *
 * \ingroup Core
 */
class MITKCORE_EXPORT AnatomicalStructureColorPresets : public vtkXMLParser
{
public:
  /** \brief Represents a DICOM coding scheme category entry. */
  struct Category
  {
    std::string codeValue;
    std::string codeScheme;
    std::string codeName;
    Category() = default;
    Category(std::string value, std::string scheme, std::string name) : codeValue(value), codeScheme(scheme), codeName(name){}
  };

  /** \brief Represents a DICOM coding scheme type entry with an optional modifier. */
  struct Type
  {
    /** \brief Represents a DICOM coding scheme modifier for a type. */
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

  /** \brief Load the default presets from the module resource XML file.
   * \return True if the XML file was parsed successfully.
   */
  bool LoadPreset();

  /** \brief Load presets from the specified XML file.
   * \param fileName Path to the XML preset file.
   * \return True if the XML file was parsed successfully.
   */
  bool LoadPreset(const std::string& fileName);

  /** \brief Get the category for the given preset name.
   * \param name The preset name.
   * \return The Category for the given name.
   */
  Category GetCategory(const std::string& name);

  /** \brief Get the type for the given preset name.
   * \param name The preset name.
   * \return The Type for the given name.
   */
  Type GetType(const std::string& name);

  /** \brief Get the color for the given preset name.
   * \param name The preset name.
   * \return The Color for the given name.
   */
  Color GetColor(const std::string& name);

  /** \brief Get all category presets.
   * \return A map from preset names to Category objects.
   */
  std::map<std::string, Category> const GetCategoryPresets();

  /** \brief Get all type presets.
   * \return A map from preset names to Type objects.
   */
  std::map<std::string, Type> const GetTypePresets();

  /** \brief Get all color presets.
   * \return A map from preset names to Color objects.
   */
  std::map<std::string, Color> const GetColorPresets();

  /** \brief Replace all presets with new ones and save.
   * \param newCategory The new category presets map.
   * \param newType The new type presets map.
   * \param newColor The new color presets map.
   */
  void NewPresets(std::map<std::string, Category>& newCategory, std::map<std::string, Type>& newType, std::map<std::string, Color>& newColor);

protected:
  /** \brief Default constructor. */
  AnatomicalStructureColorPresets() = default;
  /** \brief Destructor. */
  ~AnatomicalStructureColorPresets() override = default;

private:
  /** \brief Callback invoked when a start-tag is encountered during XML parsing. */
  void StartElement (const char *elementName, const char **atts) override;

  /** \brief Save the current presets (not yet implemented). */
  void Save();

  /** \brief Read a string attribute from an XML element's attribute list.
   * \param name The name of the attribute to read.
   * \param atts The null-terminated array of attribute name/value pairs.
   * \return The attribute value, or an empty string if not found.
   */
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
