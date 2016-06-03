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
    * Persistent properties will be saved if the file format supports custom key value pairs like the .nrrd file format.
    */
  class MITKCORE_EXPORT IPropertyPersistence
  {
  public:
    virtual ~IPropertyPersistence();

    using InfoMapType = std::multimap <std::string, PropertyPersistenceInfo::Pointer >;
    using MimeTypeNameType = PropertyPersistenceInfo::MimeTypeNameType;

    /** \brief Add persistence info for a specific base data property.
      * If there is already a property info instances for the passed
      * property name and the same info, it won't be added. Infos are
      * regarded equal, if the mime types are equal.
      * You may enforce to overrite the old equal info for a property name
      * by the overweite parameter.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] info Persistence info of the property.
      * \param[in] overwrite Overwrite already existing persistence info.
      * \return True if persistence info was added successfully.
      */
    virtual bool AddInfo(const std::string& propertyName, PropertyPersistenceInfo::Pointer info, bool overwrite = false) = 0;

    /** \brief Get the persistence info for a specific base data property.
      *
      * \param[in] propertyName Name of the property.
      * \return Property persistence info or null pointer if no persistence info is available.
      */
    virtual InfoMapType GetInfos(const std::string& propertyName) = 0;

   /** \brief Get the persistence info that will use the specified key.
    *
    * \param[in] propertyName Name of the property.
    * \return Property persistence info or null pointer if no persistence info is available.
    */
    virtual InfoMapType GetInfosByKey(const std::string& persistenceKey) = 0;

    /** \brief Get the persistence info for a specific base data property and mime type.
    *
    * \param[in] propertyName Name of the property.
    * \param[in] mime Name of the mime type the info is specified for.
    * \param[in] allowWildCard Indicates if wildcard is allowed. If it is allowed, the method will first try to find the specified info.
    * If no info was found but an info exists with the mime type name PropertyPersistenceInfo::ANY_MIMETYPE_NAME(), the later info will be
    * returned as fall back option.
    * \return Property persistence info or null pointer if no persistence info is available.
    */
    virtual PropertyPersistenceInfo::Pointer GetInfo(const std::string& propertyName, const MimeTypeNameType& mime, bool allowWildCard = false) = 0;

    /** \brief Check if a specific base data property has persistence info.
      *
      * \param[in] propertyName Name of the property.
      * \return True if the property has persistence info, false otherwise.
      */
    virtual bool HasInfos(const std::string& propertyName) = 0;

    /** \brief Remove all persistence info.
      */
    virtual void RemoveAllInfos() = 0;

    /** \brief Remove persistence infos of a specific property name.
      *
      * \param[in] propertyName Name of the property.
      */
    virtual void RemoveInfos(const std::string& propertyName) = 0;

    /** \brief Remove persistence infos of a specific property name and mime type.
    *
    * \param[in] propertyName Name of the property.
    * \param[in] mime Name of the mime type.
    */
    virtual void RemoveInfos(const std::string& propertyName, const MimeTypeNameType& mime) = 0;

  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyPersistence, "org.mitk.IPropertyPersistence")

#endif
