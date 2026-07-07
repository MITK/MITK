/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVectorPropertySerializer_h
#define mitkVectorPropertySerializer_h

#include <mitkBasePropertySerializer.h>
#include <mitkVectorProperty.h>
#include <mitkLexicalCast.h>
#include <tinyxml2.h>

namespace mitk
{
  /**
   * \brief Serializes and deserializes VectorProperty instances to/from XML.
   *
   * Converts a VectorProperty (a vector of values of type DATATYPE) into an
   * XML structure and back. The XML format is:
   *
   * \verbatim
   * <Values>
   *   <Value idx="0" value="17.3"/>
   *   <Value idx="1" value="7.2"/>
   *   <Value idx="2" value="-17.3"/>
   * </Values>
   * \endverbatim
   *
   * This class is implemented as a template and uses boost::lexical_cast
   * for conversions between specific data types and strings.
   *
   * \tparam DATATYPE The element type stored in the VectorProperty (e.g., double, int).
   *
   * \note The class name returned by GetStaticNameOfClass() is prefixed with
   *       a type-dependent string (e.g., "Double" or "Int"), so that the ITK
   *       object factory can discover the correct serializer for each VectorProperty
   *       specialization.
   *
   * \sa BasePropertySerializer, VectorProperty, DoubleVectorPropertySerializer,
   *     IntVectorPropertySerializer
   */
  template <typename DATATYPE>
  class MITKSCENESERIALIZATIONBASE_EXPORT VectorPropertySerializer : public BasePropertySerializer
  {
  public:
    // Expand manually most of mitkClassMacro:
    // mitkClassMacro(VectorProperty<DATATYPE>, mitk::BaseProperty);
    // This manual expansion is done to override explicitly
    // the GetNameOfClass methods
    typedef VectorProperty<DATATYPE> PropertyType;
    typedef VectorPropertySerializer<DATATYPE> Self;
    typedef BasePropertySerializer SuperClass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    std::vector<std::string> GetClassHierarchy() const override { return mitk::GetClassHierarchy<Self>(); }
    // This function must return different
    // strings in function of the template parameter!
    // Serialization depends on this feature.
    static const char *GetStaticNameOfClass()
    {
      // concatenate a prefix dependent on the template type and our own classname
      static std::string nameOfClass =
        std::string(VectorPropertyDataType<DATATYPE>::prefix()) + "VectorPropertySerializer";
      return nameOfClass.c_str();
    }

    const char *GetNameOfClass() const override { return this->GetStaticNameOfClass(); }
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Serializes the VectorProperty into an XML "Values" element.
     *
     * Each element of the vector is stored as a child element with an
     * "idx" attribute (zero-based index) and a "value" attribute (string
     * representation of the numeric value).
     *
     * \param[in,out] doc The XML document used to create elements.
     * \return Pointer to the "Values" XML element, or nullptr if the
     *         property is not a VectorProperty of the expected type.
     */
    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      auto *listElement = doc.NewElement("Values");

      if (const PropertyType *prop = dynamic_cast<const PropertyType *>(m_Property.GetPointer()))
      {
        typename PropertyType::VectorType elements = prop->GetValue();
        unsigned int index(0);
        for (auto listEntry : elements)
        {
          std::stringstream indexS;
          indexS << index++;

          auto *entryElement = doc.NewElement("Value");
          entryElement->SetAttribute("idx", indexS.str().c_str());
          entryElement->SetAttribute("value", boost::lexical_cast<std::string>(listEntry).c_str());
          listElement->InsertEndChild(entryElement);
        }

        return listElement;
      }
      else
      {
        return nullptr;
      }
    }

    /**
     * \brief Deserializes an XML "Values" element back into a VectorProperty.
     *
     * Reads child "Value" elements, parses each "value" attribute using
     * boost::lexical_cast, and constructs a new VectorProperty containing
     * the parsed values.
     *
     * \param[in] listElement The XML "Values" element to deserialize. May be nullptr.
     * \return A smart pointer to the deserialized VectorProperty, or nullptr
     *         if the element is null, missing, or contains unparsable values.
     */
    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *listElement) override
    {
      typename PropertyType::VectorType datalist;

      if (listElement)
      {
        std::string valueString;
        DATATYPE value;
        for (auto *valueElement = listElement->FirstChildElement("Value"); valueElement;
             valueElement = valueElement->NextSiblingElement("Value"))
        {
          valueString = valueElement->Attribute("value");
          if (valueString.empty())
          {
            MITK_ERROR << "Missing value attribute in <Values> list";
            return nullptr;
          }

          try
          {
            value = boost::lexical_cast<DATATYPE>(valueString);
          }
          catch (boost::bad_lexical_cast &e)
          {
            MITK_ERROR << "Could not parse '" << valueString << "' as number: " << e.what();
            return nullptr;
          }

          datalist.push_back(value);
        }

        typename PropertyType::Pointer property = PropertyType::New();
        property->SetValue(datalist);
        return property.GetPointer();
      }
      else
      {
        MITK_ERROR << "Missing <Values> tag.";
      }

      return nullptr;
    }
  };

  /** \brief Convenience typedef for a VectorPropertySerializer specialized for double values. */
  typedef VectorPropertySerializer<double> DoubleVectorPropertySerializer;
  /** \brief Convenience typedef for a VectorPropertySerializer specialized for int values. */
  typedef VectorPropertySerializer<int> IntVectorPropertySerializer;

} // namespace

#endif
