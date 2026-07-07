/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionPropertySerializer_h
#define mitkTransferFunctionPropertySerializer_h

#include <mitkBasePropertySerializer.h>
#include <mitkTransferFunctionProperty.h>

namespace mitk
{
  /**
   * \brief Serializer for mitk::TransferFunctionProperty.
   *
   * Serializes and deserializes a TransferFunctionProperty to and from an XML
   * representation. The XML structure contains three sub-elements:
   * - ScalarOpacity: piecewise function for scalar opacity mapping
   * - GradientOpacity: piecewise function for gradient opacity mapping
   * - Color: color transfer function with RGB values, midpoints, and sharpness
   *
   * Additionally provides static convenience methods for serializing/deserializing
   * transfer functions directly to/from files.
   *
   * \sa BasePropertySerializer, TransferFunctionProperty, TransferFunction
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT TransferFunctionPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(TransferFunctionPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Serializes the TransferFunctionProperty into an XML element.
     *
     * Creates a "TransferFunction" XML element with sub-elements for
     * ScalarOpacity, GradientOpacity, and Color transfer function data.
     *
     * \param[in,out] doc The XML document used to create elements.
     * \return Pointer to the created XML element, or nullptr if the property
     *         is not a TransferFunctionProperty or the transfer function is null.
     */
    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override;

    /**
     * \brief Deserializes an XML element back into a TransferFunctionProperty.
     *
     * Reconstructs scalar opacity, gradient opacity, and color transfer
     * functions from the XML element.
     *
     * \param[in] element The XML element to deserialize. May be nullptr.
     * \return A smart pointer to the deserialized TransferFunctionProperty,
     *         or nullptr if the element is null or parsing fails.
     *
     * \throw boost::bad_lexical_cast If numeric string values cannot be parsed.
     */
    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override;

    /**
     * \brief Serializes a TransferFunction to an XML file.
     *
     * Convenience method that wraps the transfer function in a
     * TransferFunctionProperty and writes the XML representation to the
     * specified file, including a version header.
     *
     * \param[in] filename Path to the output XML file.
     * \param[in] tf       The transfer function to serialize.
     * \return True if the file was written successfully, false otherwise.
     */
    static bool SerializeTransferFunction(const char *filename, TransferFunction::Pointer tf);

    /**
     * \brief Deserializes a TransferFunction from an XML file.
     *
     * Convenience method that reads and parses the XML file at the given
     * path, and returns the reconstructed TransferFunction.
     *
     * \param[in] filePath Path to the XML file containing the transfer function.
     * \return A smart pointer to the deserialized TransferFunction,
     *         or nullptr if reading or parsing fails.
     */
    static TransferFunction::Pointer DeserializeTransferFunction(const char *filePath);

  protected:
    TransferFunctionPropertySerializer();
    ~TransferFunctionPropertySerializer() override;
  };
}

#endif
