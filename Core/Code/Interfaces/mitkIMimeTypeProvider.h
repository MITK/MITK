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

#ifndef MITKIMIMETYPEPROVIDER_H
#define MITKIMIMETYPEPROVIDER_H

#include <MitkCoreExports.h>

#include <mitkMimeType.h>

#include <mitkServiceInterface.h>
#include <usServiceReference.h>

#include <vector>

namespace mitk {

/**
 * @brief The IMimeTypeProvider service interface allows to query all registered
 *        mime types.
 *
 * Mime types are added to the system by registering a service object of type
 * CustomMimeType and the registered mime types can be queried bei either using direct
 * look-ups in the service registry or calling the methods of this service interface.
 *
 * This service interface also allows to infer the mime type of a file on the file
 * system. The heuristics for infering the actual mime type is implementation specific.
 *
 * @note This is a <em>core service</em>
 *
 * @sa CustomMimeType
 * @sa CoreServices::GetMimeTypeProvider()
 */
struct MITK_CORE_EXPORT IMimeTypeProvider
{

  virtual ~IMimeTypeProvider();

  virtual std::vector<MimeType> GetMimeTypes() const = 0;

  virtual std::vector<MimeType> GetMimeTypesForFile(const std::string& filePath) const = 0;

  virtual std::vector<MimeType> GetMimeTypesForCategory(const std::string& category) const = 0;

  virtual MimeType GetMimeTypeForName(const std::string& name) const = 0;

  /**
   * @brief Get a sorted and unique list of mime-type categories.
   * @return A sorted, unique list of mime-type categories.
   */
  virtual std::vector<std::string> GetCategories() const = 0;
};

}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IMimeTypeProvider, "org.mitk.IMimeTypeProvider")

#endif // MITKIMIMETYPEPROVIDER_H
