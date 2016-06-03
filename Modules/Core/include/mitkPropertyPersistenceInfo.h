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

#ifndef mitkPropertyPersistenceInfo_h
#define mitkPropertyPersistenceInfo_h

#include <functional>

#include <mitkCommon.h>
#include <mitkBaseProperty.h>

#include <itkObjectFactory.h>
#include <MitkCoreExports.h>

namespace mitk
{

  /** \brief Property persistence info.
    This class is used to specify the way the persistance of a property of BaseData derived instances is handled.
    The info specifies the key for property, as well as the mime type the info is defined for and should be used.
    Additionally the functions for deserialization and serialization of the property can be defined.
    As default
    */
  class MITKCORE_EXPORT PropertyPersistenceInfo : public itk::LightObject
  {
  public:

    using DeserializationFunctionType = std::function < mitk::BaseProperty::Pointer(const std::string&) > ;
    using SerializationFunctionType = std::function < std::string(const mitk::BaseProperty*) > ;
    using MimeTypeNameType = std::string;

    mitkClassMacroItkParent(PropertyPersistenceInfo, itk::LightObject);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(Self, const std::string&);
    mitkNewMacro2Param(Self, const std::string&, const std::string&);

    std::string GetKey() const;
    void SetKey(const std::string& key);

    const MimeTypeNameType GetMimeTypeName() const;
    void SetMimeTypeName(const MimeTypeNameType& mimeTypeName);

    const DeserializationFunctionType GetDeserializationFunction() const;
    void SetDeserializationFunction(const DeserializationFunctionType& fnc);

    const SerializationFunctionType GetSerializationFunction() const;
    void SetSerializationFunction(const SerializationFunctionType& fnc);

    /** This mime type name indicates that a info can be used for any mime type as long as
     not another info with a more specific mime type is available.*/
    static MimeTypeNameType ANY_MIMETYPE_NAME();

  protected:
    /** \brief Constructor.
      *
      * \param[in] key Key used to save the property value as key value pair. If empty, the property name is used.
      */
    PropertyPersistenceInfo(const std::string& key = "");

    /** \brief Constructor.
    *
    * \param[in] key Key used to save the property value as key value pair.
    * \param[in] mimeTypeName mime type the info is defined for.
    */
    PropertyPersistenceInfo(const std::string& key, const std::string& mimeTypeName);
    virtual ~PropertyPersistenceInfo();

  private:
    PropertyPersistenceInfo(const Self& other);
    Self& operator=(const Self& other);

    struct Impl;
    Impl* m_Impl;
  };


  namespace PropertyPersistenceSerialization
  {
    /** Simple default serialization that uses prop->GetValueAsString for the serialization.*/
    MITKCORE_EXPORT ::std::string serializeByGetValueAsString(const mitk::BaseProperty* prop);
  }

  namespace PropertyPersistenceDeserialization
  {
    /** Simple default functions that puts the passed string into a string property.*/
    MITKCORE_EXPORT mitk::BaseProperty::Pointer deserializeToStringProperty(const std::string& value);
  }
}

#endif
