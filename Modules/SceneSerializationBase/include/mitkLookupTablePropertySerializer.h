/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLookupTablePropertySerializer_h
#define mitkLookupTablePropertySerializer_h

#include <mitkBasePropertySerializer.h>
#include <MitkSceneSerializationBaseExports.h>

namespace mitk
{
  /**
   * \brief Serializer for mitk::LookupTableProperty.
   *
   * Serializes and deserializes a LookupTableProperty (wrapping a vtkLookupTable)
   * to and from an XML representation. The XML structure includes the lookup table's
   * number of colors, scale, ramp, various ranges (hue, value, saturation, alpha,
   * table), and the complete RGBA color table.
   *
   * \sa BasePropertySerializer, LookupTableProperty, LookupTable
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT LookupTablePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(LookupTablePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * \brief Serializes the LookupTableProperty into an XML element.
     *
     * Creates an XML element with tag "LookupTable" containing the table
     * configuration (NumberOfColors, Scale, Ramp), range sub-elements
     * (HueRange, ValueRange, SaturationRange, AlphaRange, TableRange),
     * and the full RGBA color table.
     *
     * \param[in,out] doc The XML document used to create elements.
     * \return Pointer to the created XML element, or nullptr if the property
     *         is not a LookupTableProperty or the lookup table is null.
     */
    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc) override;

    /**
     * \brief Deserializes an XML element back into a LookupTableProperty.
     *
     * Reconstructs a vtkLookupTable from the XML structure and wraps it
     * in a new LookupTableProperty.
     *
     * \param[in] element The XML element to deserialize. May be nullptr.
     * \return A smart pointer to the deserialized LookupTableProperty,
     *         or nullptr if the element is null or parsing fails.
     */
    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override;

  protected:
    LookupTablePropertySerializer(){};
    ~LookupTablePropertySerializer() override{};
  };
}
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LookupTablePropertySerializer);

#endif
