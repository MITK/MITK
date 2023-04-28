/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelIOHelper_h
#define mitkMultiLabelIOHelper_h

#include <mitkLabelSet.h>

#include <itkSmartPointer.h>
#include <nlohmann/json.hpp>

#include <MitkMultilabelExports.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace itk
{
  class MetaDataDictionary;
}

namespace mitk
{
  class LabelSetImage;

  const constexpr char* const PROPERTY_NAME_TIMEGEOMETRY_TYPE = "org.mitk.timegeometry.type";
  const constexpr char* const PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS = "org.mitk.timegeometry.timepoints";
  const constexpr char* const PROPERTY_KEY_TIMEGEOMETRY_TYPE = "org_mitk_timegeometry_type";
  const constexpr char* const PROPERTY_KEY_TIMEGEOMETRY_TIMEPOINTS = "org_mitk_timegeometry_timepoints";
  const constexpr char* const PROPERTY_KEY_UID = "org_mitk_uid";

  /**
   * @brief The MultiLabelIOHelper is a static helper class that supports serialization of mitk::LabelSetImage
   *
   * This class provides static functions for converting mitk::Label into XML and also allows the serialization
   * of mitk::LabelSet as presets
   */
  class MITKMULTILABEL_EXPORT MultiLabelIOHelper
  {
  public:
    /**
     * @brief Saves the mitk::LabelSet configuration of inputImage to presetFilename.
     * The preset is stored as "*.lsetp"
     * @param presetFilename the filename including the filesystem path
     * @param inputImage the input image from which the preset should be generated
     * @return true if the serialization was successful and false otherwise
     */
    static bool SaveLabelSetImagePreset(const std::string &presetFilename,
                                        const mitk::LabelSetImage *inputImage);

    /**
     * @brief Loads an existing preset for a mitk::LabelSetImage from presetFilename and applies it to inputImage
     * @param presetFilename the filename of the preset including the filesystem path
     * @param inputImage the image to which the loaded preset will be applied
     * @return true if the deserilization was successful and false otherwise
     */
    static bool LoadLabelSetImagePreset(const std::string &presetFilename,
                                        mitk::LabelSetImage *inputImage);

    /**
     * @brief Creates a mitk::Label from an XML element
     * @param labelElem the xml element from which a mitk::Label will be created
     * @return the created mitk::Label
     */
    static itk::SmartPointer<mitk::Label> LoadLabelFromXMLDocument(const tinyxml2::XMLElement *labelElem);

    /**
     * @brief Creates an XML element from a mitk::Label
     * @param doc
     * @param label the mitk::Label from which the xml element will be created
     * @return the created XML element
     */
    static tinyxml2::XMLElement *GetLabelAsXMLElement(tinyxml2::XMLDocument &doc, Label *label);

    /**
     * @brief Since a mitk::Label is basically a mitk::PropertyList this function coverts the label's properties into
     * XML
     * @param doc
     * @param key the property's key which will be used in the XML element
     * @param property the mitk::BaseProperty that should be converted
     * @return the created XML element
     */
    static tinyxml2::XMLElement *PropertyToXMLElement(tinyxml2::XMLDocument& doc, const std::string &key, const BaseProperty *property);

    /**
     * @brief Since a mitk::Label is basically a mitk::PropertyList this function coverts a XML element into a property
     * @param key the property's key
     * @param prop the mitk::BaseProperty that will be created
     * @param elem the XML elem from which the property will be created
     * @return true if the conversion was successful and false otherwise
     */
    static bool PropertyFromXMLElement(std::string &key, itk::SmartPointer<mitk::BaseProperty> &prop, const tinyxml2::XMLElement *elem);

    /** Helper that extracts the value of a key in a meta dictionary as int.
    * If the key does not exist 0 is returned.*/
    static int GetIntByKey(const itk::MetaDataDictionary& dic, const std::string& key);
    /** Helper that extracts the value of a key in a meta dictionary as string.
    * If the key does not exist an empty string is returned.*/
    static std::string GetStringByKey(const itk::MetaDataDictionary& dic, const std::string& key);


    static nlohmann::json SerializeMultLabelGroupsToJSON(const mitk::LabelSetImage* inputImage);

    static std::vector<LabelSet::Pointer> DeserializeMultiLabelGroupsFromJSON(const nlohmann::json& listOfLabelSets);

    static nlohmann::json SerializeLabelToJSON(const Label* label);

    static mitk::Label::Pointer DeserializeLabelFromJSON(const nlohmann::json& labelJson);

  private:
    MultiLabelIOHelper();
  };
}

#endif
