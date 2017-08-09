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

#ifndef mitkIPropertyPersistence_h
#define mitkIPropertyPersistence_h

#include <mitkPropertyPersistenceInfo.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  /** \ingroup MicroServices_Interfaces
    * \brief Interface of property persistence service
    *
    * This service allows you to manage persistence info for base data properties.
    * Persistent properties will be saved if the file format supports custom key value pairs like the .nrrd file
   * format.\n
    * \remark The service supports the use of regular expressions (regex; ECMAscript syntax)
    * for property names and persistance keys to specifiy persistence rules in a generic way. The getter of the
   * interface
    * (GetInfo and GetInfoByKey) will always ensure that the return containes no regex, thus name and key in the results
    * can directly be used. For details on how the result names and keys are determined, please @TODO
    *
    */
  class MITKCORE_EXPORT IPropertyPersistence
  {
  public:
    virtual ~IPropertyPersistence();

    using InfoResultType = std::list<PropertyPersistenceInfo::ConstPointer>;
    using MimeTypeNameType = PropertyPersistenceInfo::MimeTypeNameType;

    /** \brief Add persistence info for a specific base data property.
      * If there is already a property info instance for the passed
      * property name and the same info, it won't be added. Info instances
      * are regarded equal, if the mime types are equal.
      * You may enforce to overwrite the old equal info for a property name
      * by the overwrite parameter.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] info Persistence info of the property.
      * \param[in] overwrite Overwrite already existing persistence info.
      * \return True if persistence info was added successfully.
      */
    virtual bool AddInfo(const PropertyPersistenceInfo *info, bool overwrite = false) = 0;

    /** \brief Get the persistence info for a specific base data property.
      * \param[in] propertyName Name of the property.
      * \param[in] allowNameRegEx Indicates if also added info instances with regexs are being checked.
      * \return Property persistence info or null pointer if no persistence info is available.
      */
    virtual InfoResultType GetInfo(const std::string &propertyName, bool allowNameRegEx = true) const = 0;

    /** \brief Get the persistence info for a specific base data property and mime type.
    *
    * \param[in] propertyName Name of the property.
    * \param[in] mime Name of the mime type the info is specified for.
    * \param[in] allowMimeWildCard Indicates if wildcard is allowed. If it is allowed, the method will first try to find
    * the specified info.
    * If no info was found but an info exists with the mime type name PropertyPersistenceInfo::ANY_MIMETYPE_NAME(), the
    * later info will be
    * returned as fall back option.
    * \param[in] allowNameRegEx Indicates if also added info instances with regexs are being checked.
    * \return Property persistence info or null pointer if no persistence info is available.
    */
    virtual InfoResultType GetInfo(const std::string &propertyName,
                                   const MimeTypeNameType &mime,
                                   bool allowMimeWildCard = false,
                                   bool allowNameRegEx = true) const = 0;

    /** \brief Get the persistence info that will use the specified key.
    *
    * \param[in] propertyName Name of the property.
    * \param[in] allowKeyRegEx Indicates if also added info instances with regexs for the key are being checked.
    * \return Property persistence info or null pointer if no persistence info is available.
    */
    virtual InfoResultType GetInfoByKey(const std::string &persistenceKey, bool allowKeyRegEx = true) const = 0;

    /** \brief Check if a specific base data property has persistence info.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] allowNameRegEx Indicates if also added info instances with regexs are being checked.
      * \return True if the property has persistence info, false otherwise.
      */
    virtual bool HasInfo(const std::string &propertyName, bool allowNameRegEx = true) const = 0;

    /** \brief Remove all persistence info.
      */
    virtual void RemoveAllInfo() = 0;

    /** \brief Remove persistence info instances of a specific property name/regex.
      *
      * \param[in] propertyName Registered name or regex that should be removed.
      */
    virtual void RemoveInfo(const std::string &propertyName) = 0;

    /** \brief Remove persistence info instances of a specific property name/regex and mime type.
    *
    * \param[in] propertyName Registered name or regex that should be removed.
    * \param[in] mime Name of the mime type.
    */
    virtual void RemoveInfo(const std::string &propertyName, const MimeTypeNameType &mime) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyPersistence, "org.mitk.IPropertyPersistence")

#endif
