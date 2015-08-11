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

    /** \brief Add persistence info for a specific base data property.
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
    virtual PropertyPersistenceInfo::Pointer GetInfo(const std::string& propertyName) = 0;

    /** \brief Check if a specific base data property has persistence info.
      *
      * \param[in] propertyName Name of the property.
      * \return True if the property has persistence info, false otherwise.
      */
    virtual bool HasInfo(const std::string& propertyName) = 0;

    /** \brief Remove all persistence info.
      */
    virtual void RemoveAllInfos() = 0;

    /** \brief Remove persistence info of a specific base data property.
      *
      * \param[in] propertyName Name of the property.
      */
    virtual void RemoveInfo(const std::string& propertyName) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyPersistence, "org.mitk.IPropertyPersistence")

#endif
