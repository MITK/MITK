/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMimeTypeProvider_h
#define mitkMimeTypeProvider_h

#include <mitkCustomMimeType.h>
#include <mitkIMimeTypeProvider.h>

#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>

#include <set>

namespace mitk
{
  /**
   * \brief Custom traits for tracking MimeType objects via CppMicroServices ServiceTracker.
   *
   * Defines validity, default value, and disposal semantics for tracked MimeType instances.
   */
  struct MimeTypeTrackerTypeTraits : public us::TrackedTypeTraitsBase<MimeType, MimeTypeTrackerTypeTraits>
  {
    typedef MimeType TrackedType;

    static bool IsValid(const TrackedType &t) { return t.IsValid(); }
    static TrackedType DefaultValue() { return TrackedType(); }
    static void Dispose(TrackedType & /*t*/) {}
  };

  /**
   * \brief Implementation of IMimeTypeProvider that tracks MIME type services.
   *
   * Uses a CppMicroServices ServiceTracker to dynamically discover and manage
   * CustomMimeType service registrations. For each MIME type name, keeps track
   * of all registered variants and provides the highest-ranked one. This class
   * must be started (Start()) before use and stopped (Stop()) before destruction.
   *
   * \sa IMimeTypeProvider
   * \sa CustomMimeType
   * \sa MimeType
   */
  class MimeTypeProvider : public IMimeTypeProvider, private us::ServiceTrackerCustomizer<CustomMimeType, MimeType>
  {
  public:
    MimeTypeProvider();
    ~MimeTypeProvider() override;

    /**
     * \brief Start tracking MIME type services.
     *
     * Opens the internal ServiceTracker. Must be called before any query methods.
     */
    void Start();

    /**
     * \brief Stop tracking MIME type services.
     *
     * Closes the internal ServiceTracker. Must be called before destruction.
     */
    void Stop();

    /**
     * \brief Get all currently registered MIME types (highest-ranked per name).
     * \return A vector of all known MimeType objects.
     */
    std::vector<MimeType> GetMimeTypes() const override;

    /**
     * \brief Get all MIME types that match the given file path.
     *
     * Results are sorted by rank in descending order (highest rank first).
     *
     * \param[in] filePath The file path to match against.
     * \return A vector of matching MimeType objects.
     */
    std::vector<MimeType> GetMimeTypesForFile(const std::string &filePath) const override;

    /**
     * \brief Get all MIME types belonging to the given category.
     * \param[in] category The category to filter by.
     * \return A vector of matching MimeType objects.
     */
    std::vector<MimeType> GetMimeTypesForCategory(const std::string &category) const override;

    /**
     * \brief Get the highest-ranked MIME type with the given name.
     * \param[in] name The MIME type name.
     * \return The MimeType, or an invalid MimeType if not found.
     */
    MimeType GetMimeTypeForName(const std::string &name) const override;

    /**
     * \brief Get all unique MIME type categories.
     * \return A sorted vector of unique category strings.
     */
    std::vector<std::string> GetCategories() const override;

  private:
    TrackedType AddingService(const ServiceReferenceType &reference) override;
    void ModifiedService(const ServiceReferenceType &reference, TrackedType service) override;
    void RemovedService(const ServiceReferenceType &reference, TrackedType service) override;

    /**
     * \brief Create a MimeType from a service reference.
     * \param[in] reference The service reference to a CustomMimeType.
     * \return The constructed MimeType, or an invalid MimeType on failure.
     */
    MimeType GetMimeType(const ServiceReferenceType &reference) const;

    us::ServiceTracker<CustomMimeType, MimeTypeTrackerTypeTraits> *m_Tracker;

    typedef std::map<std::string, std::set<MimeType>> MapType;
    MapType m_NameToMimeTypes; ///< All registered variants per MIME type name.

    std::map<std::string, MimeType> m_NameToMimeType; ///< Highest-ranked MIME type per name.
  };
}

#endif
