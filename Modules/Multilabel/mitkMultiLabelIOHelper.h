/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelIOHelper_h
#define mitkMultiLabelIOHelper_h

#include <mitkLabel.h>

#include <mitkLabelSetImage.h>

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
  const constexpr char* const PROPERTY_NAME_TIMEGEOMETRY_TYPE = "org.mitk.timegeometry.type";
  const constexpr char* const PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS = "org.mitk.timegeometry.timepoints";
  const constexpr char* const PROPERTY_KEY_TIMEGEOMETRY_TYPE = "org_mitk_timegeometry_type";
  const constexpr char* const PROPERTY_KEY_TIMEGEOMETRY_TIMEPOINTS = "org_mitk_timegeometry_timepoints";
  const constexpr char* const PROPERTY_KEY_UID = "org_mitk_uid";

  /**
   * @brief The MultiLabelIOHelper is a static helper class that supports serialization of mitk::MultiLabelSegmentation
   *
   * This class provides static functions for converting mitk::Label into XML and also allows the serialization
   * of mitk::LabelSet as presets
   */
  class MITKMULTILABEL_EXPORT MultiLabelIOHelper
  {
  public:
    /**
     * @brief Saves the mitk::LabelSet configuration of inputSegmentation to presetFilename.
     * The preset is stored as "*.lsetp"
     * @param presetFilename the filename including the filesystem path
     * @param inputSegmentation the input image from which the preset should be generated
     * @return true if the serialization was successful and false otherwise
     */
    static bool SaveLabelSetImagePreset(const std::string &presetFilename,
                                        const mitk::MultiLabelSegmentation *inputImage);

    /**
     * @brief Loads an existing preset for a mitk::MultiLabelSegmentation from presetFilename and applies it to inputSegmentation
     * @param presetFilename the filename of the preset including the filesystem path
     * @param inputSegmentation the image to which the loaded preset will be applied
     * @return true if the deserilization was successful and false otherwise
     */
    static bool LoadLabelSetImagePreset(const std::string &presetFilename,
                                        mitk::MultiLabelSegmentation *inputImage);

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
    static tinyxml2::XMLElement *GetLabelAsXMLElement(tinyxml2::XMLDocument &doc, const Label *label);

    /**
     * @brief Since a mitk::Label is basically a mitk::PropertyList this function converts the label's properties into
     * XML
     * @param doc
     * @param key the property's key which will be used in the XML element
     * @param property the mitk::BaseProperty that should be converted
     * @return the created XML element
     */
    static tinyxml2::XMLElement *PropertyToXMLElement(tinyxml2::XMLDocument& doc, const std::string &key, const BaseProperty *property);

    /**
     * @brief Since a mitk::Label is basically a mitk::PropertyList this function converts an XML element into a property
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

    /**
     * @brief Structure to hold label group metadata including labels and group properties
     */
    struct MITKMULTILABEL_EXPORT LabelGroupMetaData
    {
      std::string name;
      LabelVector labels;
      PropertyList::Pointer properties; // For custom group properties

      LabelGroupMetaData();
      LabelGroupMetaData(const std::string& groupName, const LabelVector& groupLabels);
      LabelGroupMetaData(const std::string& groupName, const LabelVector& groupLabels, PropertyList* groupProperties);
      ~LabelGroupMetaData() = default;
    };


    using GroupFileNameCallback = std::function<std::string(const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::GroupIndexType)>;
    using LabelFileNameCallback = std::function<std::string(const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::LabelValueType)>;
    using LabelFileValueCallback = std::function<mitk::MultiLabelSegmentation::LabelValueType(const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation::LabelValueType)>;

    /**
     * @brief Serialize meta data all groups of the multilabel segmentation to JSON format (v2)
     * @param inputSegmentation the multilabel segmentation to serialize
     * @param groupFileNameCallback Optional call back function. If provided the property "file" on group level
     * based on the returned string will be set.
     * @param labelFileNameCallback Optional call back function. If provided the property "file" per label
     * based on the returned string will be set.
     * @param labelFileValueCallback Optional call back function. If provided the property "file_value" per label
     * will be set according to the returned value.
     * @return JSON representation of the groups
     */
    static nlohmann::json SerializeMultLabelGroupsToJSON(const mitk::MultiLabelSegmentation* inputSegmentation,
      GroupFileNameCallback groupFileNameCallback = nullptr ,
      LabelFileNameCallback labelFileNameCallback = nullptr,
      LabelFileValueCallback labelFileValueCallback = nullptr);

    /**
     * @brief Deserialize meta data of multilabel groups from JSON format (v2)
     * @param listOfLabelGroups JSON representation of label groups
     * @return vector of label group metadata
     */
    static std::vector<LabelGroupMetaData> DeserializeMultiLabelGroupsFromJSON(const nlohmann::json& listOfLabelSets);

    /**
     * @brief Serialize a single label to JSON
     * @param label the label to serialize
     * @return JSON representation of the label
     */
    static nlohmann::json SerializeLabelToJSON(const Label* label);

    /**
     * @brief Deserialize a single label from JSON
     * @param labelJson JSON representation of the label
     * @return pointer to the deserialized label
     */
    static mitk::Label::Pointer DeserializeLabelFromJSON(const nlohmann::json& labelJson);

    /**
     * @brief Serialize the property of a label to JSON
     * @param property Pointer to the property that should be serialized
     * @return JSON representation of the property
     */
    static nlohmann::json mitk::MultiLabelIOHelper::SerializeLabelPropertyToJSON(const BaseProperty* property);

  private:
    MultiLabelIOHelper();
  };
}

#endif
