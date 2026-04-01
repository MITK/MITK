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
  /**
   * \brief Manages level/window presets loaded from an XML file.
   *
   * Reads named level/window presets from an XML resource file
   * (mitkLevelWindowPresets.xml) and provides access to the stored values.
   *
   * \ingroup DataManagement
   */
  class MITKCORE_EXPORT LevelWindowPreset : public vtkXMLParser
  {
  public:
    static LevelWindowPreset *New();
    vtkTypeMacro(LevelWindowPreset, vtkXMLParser);

    /** \brief Load presets from the default module resource file.
     *  \return True if parsing was successful.
     */
    bool LoadPreset();

    /** \brief Load presets from a specified XML file.
     *  \param fileName Path to the XML file to load.
     *  \return True if parsing was successful.
     */
    bool LoadPreset(std::string fileName);

    /** \brief Get the level value for a named preset.
     *  \param name The name of the preset.
     *  \return The level value associated with the name.
     */
    double getLevel(std::string name);

    /** \brief Get the window value for a named preset.
     *  \param window The name of the preset.
     *  \return The window value associated with the name.
     */
    double getWindow(std::string window);

    /** \brief Get a reference to the map of all level presets.
     *  \return Map from preset name to level value.
     */
    std::map<std::string, double> &getLevelPresets();

    /** \brief Get a reference to the map of all window presets.
     *  \return Map from preset name to window value.
     */
    std::map<std::string, double> &getWindowPresets();

    /** \brief Replace all presets with new level and window maps and save.
     *  \param newLevel New map of level presets.
     *  \param newWindow New map of window presets.
     */
    void newPresets(std::map<std::string, double> newLevel, std::map<std::string, double> newWindow);

  protected:
    LevelWindowPreset();
    ~LevelWindowPreset() override;

  private:
    /** \brief Callback used in XML reading; gets called when a start-tag is read. */
    void StartElement(const char *elementName, const char **atts) override;

    /** \brief Saves presets to the XML file. */
    void save();

    /** \brief Reads an XML string attribute by name.
     *  \param name The attribute name to look for.
     *  \param atts The null-terminated array of attribute name/value pairs.
     *  \return The attribute value, or an empty string if not found.
     */
    std::string ReadXMLStringAttribut(std::string name, const char **atts);

    static const std::string PRESET;
    std::map<std::string, double> m_Level;
    std::map<std::string, double> m_Window;
    std::string m_XmlFileName;
  };
}
#endif
