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

#include <mitkBaseProperty.h>
#include <mitkCommon.h>

#include <MitkCoreExports.h>
#include <itkObjectFactory.h>

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
    using DeserializationFunctionType = std::function<mitk::BaseProperty::Pointer(const std::string &)>;
    using SerializationFunctionType = std::function<std::string(const mitk::BaseProperty *)>;
    using MimeTypeNameType = std::string;

    mitkClassMacroItkParent(PropertyPersistenceInfo, itk::LightObject);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self) mitkNewMacro1Param(Self, const std::string &);
    mitkNewMacro2Param(Self, const std::string &, const std::string &);

    std::string GetName() const;
    std::string GetKey() const;
    void SetName(const std::string &name);
    void SetNameAndKey(const std::string &name, const std::string &key);

    bool IsRegEx() const;

    /** Sets the name and the key identifier as a regular expression that describes valid names and keys.
    * @pre nameRegEx must be a valid regular expression, otherweis a regex_error esception
    * is thrown and the info object is not changed.
    * @pre keyRegEx must be a valid regular expression, otherweis a regex_error esception
    * is thrown and the info object is not changed.*/
    void UseRegEx(const std::string &nameRegEx, const std::string &nameTemplate);
    void UseRegEx(const std::string &nameRegEx,
                  const std::string &nameTemplate,
                  const std::string &keyRegEx,
                  const std::string keyTemplate);

    const std::string &GetKeyTemplate() const;
    const std::string &GetNameTemplate() const;

    const MimeTypeNameType &GetMimeTypeName() const;
    void SetMimeTypeName(const MimeTypeNameType &mimeTypeName);

    const DeserializationFunctionType GetDeserializationFunction() const;
    void SetDeserializationFunction(const DeserializationFunctionType &fnc);

    const SerializationFunctionType GetSerializationFunction() const;
    void SetSerializationFunction(const SerializationFunctionType &fnc);

    PropertyPersistenceInfo::Pointer UnRegExByName(const std::string &propertyName) const;
    PropertyPersistenceInfo::Pointer UnRegExByKey(const std::string &key) const;

    /** This mime type name indicates that a info can be used for any mime type as long as
     another info with a more specific mime type is not available.*/
    static MimeTypeNameType ANY_MIMETYPE_NAME();

  protected:
    /** \brief Constructor.
      *
      * \param[in] name Name is the name of the property that described by the info. Key will be the same.
      */
    PropertyPersistenceInfo(const std::string &name = "");

    /** \brief Constructor.
    *
    * \param[in] name Name is the name of the property that described by the info. Key will be the same.
    * \param[in] mimeTypeName mime type the info is defined for.
    */
    PropertyPersistenceInfo(const std::string &name, const std::string &mimeTypeName);

    ~PropertyPersistenceInfo() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
    PropertyPersistenceInfo(const Self &other);
    Self &operator=(const Self &other);

    struct Impl;
    Impl *m_Impl;
  };

  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const PropertyPersistenceInfo &info);

  namespace PropertyPersistenceSerialization
  {
    /** Simple default serialization that uses prop->GetValueAsString for the serialization.*/
    MITKCORE_EXPORT::std::string serializeByGetValueAsString(const mitk::BaseProperty *prop);
  }

  namespace PropertyPersistenceDeserialization
  {
    /** Simple default functions that puts the passed string into a string property.*/
    MITKCORE_EXPORT mitk::BaseProperty::Pointer deserializeToStringProperty(const std::string &value);
  }
}

#endif
