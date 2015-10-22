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

#ifndef mitkVectorPropertySerializer_h
#define mitkVectorPropertySerializer_h

#include "mitkBasePropertySerializer.h"

#include "mitkVectorProperty.h"

namespace mitk {

/**
  \brief Serializes a VectorProperty

  Serializes an instance of VectorProperty into a XML structure like

  \verbatim
  <Values>
    <Value idx="0" value="17.3"/>
    <Value idx="1" value="7.2"/>
    <Value idx="2" value="-17.3"/>
  </Values>
  \endverbatim

  This class is implemented as a template and makes use of std::stringstream
  for necessary conversions of specific data types to and from string.

  For numeric types, the class adds a precision token to stringstream that
  should usually suffice.
*/
template< typename DATATYPE >
class MITKSCENESERIALIZATIONBASE_EXPORT VectorPropertySerializer : public BasePropertySerializer
{
public:

  // Expand manually most of mitkClassMacro:
  // mitkClassMacro(VectorProperty<DATATYPE>, mitk::BaseProperty);
  // This manual expansion is done to override explicitely
  // the GetNameOfClass methods
  typedef VectorProperty<DATATYPE> PropertyType;
  typedef VectorPropertySerializer<DATATYPE> Self;
  typedef BasePropertySerializer SuperClass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;
  virtual std::vector<std::string> GetClassHierarchy() const override
  {
      return mitk::GetClassHierarchy<Self>();
  }

  // This function must return different
  // strings in function of the template parameter!
  // Serialization depends on this feature.
  static const char* GetStaticNameOfClass()
  {
    // concatenate a prefix dependent on the template type and our own classname
    static std::string nameOfClass = std::string( VectorPropertyDataType<DATATYPE>::prefix()) + "VectorPropertySerializer";
    return nameOfClass.c_str();
  }

  virtual const char* GetNameOfClass() const override
  {
    return this->GetStaticNameOfClass();
  }

  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self)

  virtual TiXmlElement* Serialize() override
  {
    auto listElement = new TiXmlElement( "Values" );

    if ( const PropertyType* prop = dynamic_cast<const PropertyType*>(m_Property.GetPointer()) )
    {
      typename PropertyType::VectorType elements = prop->GetValue();
      unsigned int index(0);
      for (auto listEntry : elements)
      {
        std::stringstream indexS;
        indexS << index++;

        std::stringstream valueS;
        valueS.precision(16);
        valueS <<  listEntry;
        auto entryElement = new TiXmlElement("Value");
        entryElement->SetAttribute("idx", indexS.str());
        entryElement->SetAttribute("value", valueS.str());
        listElement->LinkEndChild( entryElement );
      }

      return listElement;
    }
    else
    {
      return nullptr;
    }
  }


    virtual BaseProperty::Pointer Deserialize(TiXmlElement* listElement) override
    {
      typename PropertyType::VectorType datalist;

      if ( listElement )
      {
        MITK_DEBUG << "Deserializing " << *listElement;

        unsigned int index(0);
        std::string valueString;
        DATATYPE value;
        for ( TiXmlElement* valueElement = listElement->FirstChildElement("Value");
              valueElement;
              valueElement = valueElement->NextSiblingElement("Value") )
        {
          if ( valueElement->QueryValueAttribute("value", &valueString) != TIXML_SUCCESS )
          {
              MITK_ERROR << "Missing value attribute in <Values> list";
            return nullptr;
          }

          std::istringstream ss(valueString);
          if ( !(ss >> value ) )
          {
            MITK_ERROR << "Could not make sense of '" << valueString << "'";
            return nullptr;
          }

          datalist.push_back(value);
          ++index;
        }

        typename PropertyType::Pointer property = PropertyType::New();
        property->SetValue( datalist );
        return property.GetPointer();
      }
      else
      {
        MITK_ERROR << "Missing <Values> tag.";
      }

      return nullptr;
    }

};

typedef VectorPropertySerializer<double> DoubleVectorPropertySerializer;
typedef VectorPropertySerializer<int> IntVectorPropertySerializer;

} // namespace

#endif
