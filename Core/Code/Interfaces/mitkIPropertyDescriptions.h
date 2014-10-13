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

#ifndef mitkIPropertyDescriptions_h
#define mitkIPropertyDescriptions_h

#include <mitkServiceInterface.h>
#include <string>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Interface of property descriptions service.
    *
    * This service allows you to manage descriptions for properties.
    * The property view displays descriptions of selected properties (in rich text format) at its bottom.
    */
  class MITK_CORE_EXPORT IPropertyDescriptions
  {
  public:
    virtual ~IPropertyDescriptions();

    /** \brief Add a description for a specific property.
     *
     * \param[in] propertyName Name of the property.
     * \param[in] description Description of the property.
     * \param[in] className Optional data node class name to which this description is restricted.
     * \param[in] overwrite Overwrite already existing description.
     * \return True if description was added successfully.
     */
    virtual bool AddDescription(const std::string& propertyName, const std::string& description, const std::string& className = "", bool overwrite = false) = 0;

    /** \brief Get the description for a specific property.
     *
     * \param[in] propertyName Name of the property.
     * \param[in] className Optional data node class name to which the returned description is restricted.
     * \return Property description or empty string if no description is available.
     */
    virtual std::string GetDescription(const std::string& propertyName, const std::string& className = "") = 0;

    /** \brief Check if a specific property has a description.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] className Optional data node class name to which this description is restricted.
      * \return True if the property has a description, false otherwise.
      */
    virtual bool HasDescription(const std::string& propertyName, const std::string& className = "") = 0;

    /** \brief Remove all descriptions.
      *
      * \param[in] className Optional data node class name to which this description is restricted.
      */
    virtual void RemoveAllDescriptions(const std::string& className = "") = 0;

    /** \brief Remove description of specific property.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] className Optional data node class name to which this description is restricted.
      */
    virtual void RemoveDescription(const std::string& propertyName, const std::string& className = "") = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyDescriptions, "org.mitk.IPropertyDescriptions")

#endif
