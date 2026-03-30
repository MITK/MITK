/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagsOfInterestAddHelper_h
#define mitkDICOMTagsOfInterestAddHelper_h

#include <unordered_map>

#include <mitkDICOMTagPath.h>

#include <usServiceEvent.h>

#include <memory>
#include <mutex>

#include <MitkDICOMExports.h>

namespace us
{
  class ModuleContext;
}

namespace mitk
{
  class IDICOMTagsOfInterest;

  /**
   * \ingroup DICOMModule
   * \brief Helper class to ensure DICOM tags of interest are registered at IDICOMTagsOfInterest services.
   *
   * This helper class can be used to ensure that a given list of DICOM tags will be
   * registered at existing IDICOMTagsOfInterest services. If the services are available
   * the tags will be directly added. As long as the helper is active, it will add the
   * given tags to all new IDICOMTagsOfInterest services as soon as they are registered.
   *
   * This class is typically used in module activators where the IDICOMTagsOfInterest service
   * might not be registered when a module is loaded, but the module wants to ensure that
   * the tags are added as soon as possible.
   *
   * \note The helper must be deactivated before the module context used on activation
   * gets invalid (e.g. in the Unload function of the module activator).
   *
   * \sa IDICOMTagsOfInterest, DICOMTagPath
   */
  class MITKDICOM_EXPORT DICOMTagsOfInterestAddHelper
  {
  public:
    /** \brief Vector of DICOMTagPath instances representing tags to register. */
    using TagsOfInterestVector = std::vector<mitk::DICOMTagPath>;

    ~DICOMTagsOfInterestAddHelper();

    /**
     * \brief Activate the helper to start registering tags of interest.
     *
     * Registers a service listener and immediately registers tags at any existing
     * IDICOMTagsOfInterest services. Future services will also receive the tags.
     *
     * \param[in] context The CppMicroServices module context to use for service tracking.
     * \param[in] tags The list of DICOM tag paths to register.
     * \pre context must not be nullptr.
     */
    void Activate(us::ModuleContext* context, TagsOfInterestVector tags);

    /**
     * \brief Deactivate the helper and stop listening for new services.
     *
     * Must be called before the module context becomes invalid.
     */
    void Deactivate();

  private:
    void RegisterTagsOfInterest(IDICOMTagsOfInterest* toiService) const;
    void DICOMTagsOfInterestServiceChanged(const us::ServiceEvent event);

    TagsOfInterestVector m_TagsOfInterest;

    bool m_Active = false;
    us::ModuleContext* m_Context = nullptr;

    /**mutex to guard the service listening */
    std::mutex m_Mutex;
  };
}

#endif
