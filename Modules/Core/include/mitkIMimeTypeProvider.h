/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIMimeTypeProvider_h
#define mitkIMimeTypeProvider_h

#include <MitkCoreExports.h>

#include <mitkMimeType.h>

#include <mitkServiceInterface.h>
#include <usServiceReference.h>

#include <vector>

namespace mitk
{
  /**
   * \ingroup IO
   * \ingroup MicroServices_Interfaces
   *
   * \brief Service interface for querying all registered mime-types.
   *
   * Mime-types are added to the system by registering service objects of type
   * CustomMimeType. The registered mime-types can be queried either through
   * direct look-ups in the service registry or by calling the methods of
   * this service interface.
   *
   * This service interface also allows inferring the mime-type of a file on
   * the file system. The heuristics for inferring the actual mime-type is
   * implementation specific.
   *
   * \note This is a <em>core service</em> and can be obtained via
   *       CoreServices::GetMimeTypeProvider().
   *
   * \sa CustomMimeType
   * \sa MimeType
   * \sa CoreServices::GetMimeTypeProvider()
   */
  struct MITKCORE_EXPORT IMimeTypeProvider
  {
    virtual ~IMimeTypeProvider();

    /**
     * \brief Get all registered mime-types.
     * \return A vector of all registered MimeType objects.
     */
    virtual std::vector<MimeType> GetMimeTypes() const = 0;

    /**
     * \brief Get all mime-types that apply to the given file path.
     * \param[in] filePath The absolute file path to match against.
     * \return A vector of matching MimeType objects, sorted by rank (highest first).
     */
    virtual std::vector<MimeType> GetMimeTypesForFile(const std::string &filePath) const = 0;

    /**
     * \brief Get all mime-types in a specific category.
     * \param[in] category The category string (e.g. "Images", "Surfaces").
     * \return A vector of MimeType objects belonging to the given category.
     */
    virtual std::vector<MimeType> GetMimeTypesForCategory(const std::string &category) const = 0;

    /**
     * \brief Get the mime-type registered under the given name.
     * \param[in] name The unique mime-type name (e.g. "application/vnd.mitk.image.nrrd").
     * \return The matching MimeType, or an invalid MimeType if not found.
     */
    virtual MimeType GetMimeTypeForName(const std::string &name) const = 0;

    /**
     * \brief Get a sorted and unique list of all registered mime-type categories.
     * \return A sorted, unique vector of category strings.
     */
    virtual std::vector<std::string> GetCategories() const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IMimeTypeProvider, "org.mitk.IMimeTypeProvider")

#endif
