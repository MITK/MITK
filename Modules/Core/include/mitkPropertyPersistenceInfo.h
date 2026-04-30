/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    *
    * This class is used to specify the way the persistence of a property of BaseData derived instances is handled.
    * The info specifies the key for property, as well as the mime type the info is defined for and should be used.
    * Additionally the functions for deserialization and serialization of the property can be defined.
    * By default, serialization uses BaseProperty::GetValueAsString() and deserialization creates a StringProperty.
    */
  class MITKCORE_EXPORT PropertyPersistenceInfo : public itk::LightObject
  {
  public:
    /** \brief Signature specification for functions that can be provided for deserialization of the property.
     * \post The function returns a valid instance derived from mitk::BaseProperty.
     */
    using DeserializationFunctionType = std::function<mitk::BaseProperty::Pointer(const std::string &)>;

    /** \brief Signature specification for functions that can be provided for serialization of the property. */
    using SerializationFunctionType = std::function<std::string(const mitk::BaseProperty *)>;

    /** \brief Type alias for MIME type name strings. */
    using MimeTypeNameType = std::string;

    mitkClassMacroItkParent(PropertyPersistenceInfo, itk::LightObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) mitkNewMacro1Param(Self, const std::string &);
    mitkNewMacro2Param(Self, const std::string &, const std::string &);

    /** \brief Get the property name described by this info. */
    std::string GetName() const;

    /** \brief Get the persistence key used for this property.
     *
     * The key is used as identifier when the property is persisted (e.g. as DICOM tag or file key).
     * By default, the key equals the name.
     */
    std::string GetKey() const;

    /** \brief Set the property name (and key to the same value).
     *
     * Resets any regular expression settings.
     * \param[in] name The property name.
     */
    void SetName(const std::string &name);

    /** \brief Set name and key independently.
     *
     * Resets any regular expression settings.
     * \param[in] name The property name.
     * \param[in] key The persistence key.
     */
    void SetNameAndKey(const std::string &name, const std::string &key);

    /** \brief Check whether this info uses regular expressions for name and key matching. */
    bool IsRegEx() const;

    /** \brief Set the name and key as a regular expression with a shared template.
     *
     * Both name and key will use the same regex and template.
     *
     * \pre nameRegEx must be a valid regular expression, otherwise a regex_error exception
     * is thrown and the info object is not changed.
     * \param[in] nameRegEx Regular expression for matching property names.
     * \param[in] nameTemplate Template string for generating concrete names from regex groups.
     */
    void UseRegEx(const std::string &nameRegEx, const std::string &nameTemplate);

    /** \brief Set the name and key as separate regular expressions with individual templates.
     *
     * \pre nameRegEx must be a valid regular expression, otherwise a regex_error exception
     * is thrown and the info object is not changed.
     * \pre keyRegEx must be a valid regular expression, otherwise a regex_error exception
     * is thrown and the info object is not changed.
     * \param[in] nameRegEx Regular expression for matching property names.
     * \param[in] nameTemplate Template string for generating concrete names from regex groups.
     * \param[in] keyRegEx Regular expression for matching persistence keys.
     * \param[in] keyTemplate Template string for generating concrete keys from regex groups.
     */
    void UseRegEx(const std::string &nameRegEx,
                  const std::string &nameTemplate,
                  const std::string &keyRegEx,
                  const std::string keyTemplate);

    /** \brief Get the template string used for generating concrete keys from regex groups. */
    const std::string &GetKeyTemplate() const;

    /** \brief Get the template string used for generating concrete names from regex groups. */
    const std::string &GetNameTemplate() const;

    /** \brief Get the MIME type name this persistence info is associated with. */
    const MimeTypeNameType &GetMimeTypeName() const;

    /** \brief Set the MIME type name this persistence info should be associated with.
     * \param[in] mimeTypeName The MIME type name.
     */
    void SetMimeTypeName(const MimeTypeNameType &mimeTypeName);

    /** \brief Get the deserialization function used to convert a string back to a property. */
    const DeserializationFunctionType GetDeserializationFunction() const;

    /** \brief Set the deserialization function.
     * \param[in] fnc Function that converts a serialized string to a BaseProperty.
     */
    void SetDeserializationFunction(const DeserializationFunctionType &fnc);

    /** \brief Get the serialization function used to convert a property to a string. */
    const SerializationFunctionType GetSerializationFunction() const;

    /** \brief Set the serialization function.
     * \param[in] fnc Function that converts a BaseProperty to a serialized string.
     */
    void SetSerializationFunction(const SerializationFunctionType &fnc);

    /** \brief Create a non-regex info by resolving the regex against a property name.
     *
     * If this info is a regex, generates a concrete PropertyPersistenceInfo by applying
     * the name to the key template. If not a regex, returns a copy.
     * \param[in] propertyName The concrete property name to resolve against.
     * \return A new non-regex PropertyPersistenceInfo instance.
     */
    PropertyPersistenceInfo::Pointer UnRegExByName(const std::string &propertyName) const;

    /** \brief Create a non-regex info by resolving the regex against a persistence key.
     *
     * If this info is a regex, generates a concrete PropertyPersistenceInfo by applying
     * the key to the name template. If not a regex, returns a copy.
     * \param[in] key The concrete persistence key to resolve against.
     * \return A new non-regex PropertyPersistenceInfo instance.
     */
    PropertyPersistenceInfo::Pointer UnRegExByKey(const std::string &key) const;

    /** \brief Return the MIME type name that indicates compatibility with any MIME type.
     *
     * An info using this MIME type name will be used as a fallback when no
     * info with a more specific MIME type is available.
     */
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

  /** \brief Stream output operator for PropertyPersistenceInfo. */
  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const PropertyPersistenceInfo &info);

  /** \brief Namespace for default property persistence serialization functions. */
  namespace PropertyPersistenceSerialization
  {
    /** \brief Default serialization that uses BaseProperty::GetValueAsString().
     * \param[in] prop The property to serialize.
     * \return The serialized string representation, or empty string if prop is nullptr.
     */
    MITKCORE_EXPORT::std::string serializeByGetValueAsString(const mitk::BaseProperty *prop);
  }

  /** \brief Namespace for default property persistence deserialization functions. */
  namespace PropertyPersistenceDeserialization
  {
    /** \brief Default deserialization that creates a StringProperty from the passed string.
     * \param[in] value The serialized string to deserialize.
     * \return A new StringProperty containing the value.
     */
    MITKCORE_EXPORT mitk::BaseProperty::Pointer deserializeToStringProperty(const std::string &value);
  }
}

#endif
