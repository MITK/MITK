/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkLabelSetIOHelper_h
#define __mitkLabelSetIOHelper_h

#include <MitkMultilabelExports.h>
#include <itkSmartPointer.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  class BaseProperty;
  class LabelSetImage;
  class Label;

  /**
   * @brief The LabelSetIOHelper is a static helper class that supports serialization of mitk::LabelSetImage
   *
   * This class provides static functions for converting mitk::Label into XML and also allows the serialization
   * of mitk::LabelSet as presets
   */
  class MITKMULTILABEL_EXPORT LabelSetIOHelper
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
                                        itk::SmartPointer<mitk::LabelSetImage> &inputImage);

    /**
     * @brief Loads an existing preset for a mitk::LabelSetImage from presetFilename and applies it to inputImage
     * @param presetFilename the filename of the preset including the filesystem path
     * @param inputImage the image to which the loaded preset will be applied
     */
    static void LoadLabelSetImagePreset(const std::string &presetFilename,
                                        itk::SmartPointer<mitk::LabelSetImage> &inputImage);

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

  private:
    LabelSetIOHelper();
  };
}

#endif // __mitkLabelSetIOHelper_h
