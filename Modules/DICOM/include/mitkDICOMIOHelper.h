/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMIOHelper_h
#define mitkDICOMIOHelper_h

#include <mitkDICOMDatasetAccessingImageFrameInfo.h>
#include <mitkIDICOMTagsOfInterest.h>

#include <MitkDICOMExports.h>

namespace mitk
{
  class BaseData;

  /**
   * \brief Helper namespace for DICOM I/O operations.
   *
   * Provides utility functions for retrieving DICOM tags of interest services,
   * extracting tag values from frame information, and setting properties on BaseData instances.
   *
   * \ingroup DICOMModule
   * \sa IDICOMTagsOfInterest, DICOMDatasetAccess
   */
  namespace DICOMIOHelper
  {
    /** \brief Vector of findings lists, one per tag path of interest. */
    using FindingsListVectorType = std::vector<DICOMDatasetAccess::FindingsListType>;

    /**
     * \brief Retrieve the IDICOMTagsOfInterest service from the module registry.
     *
     * Queries the CppMicroServices module context for a registered IDICOMTagsOfInterest service.
     * If multiple services are registered, the first one found is returned.
     *
     * \return Pointer to the service instance, or nullptr if no service is available.
     * \sa IDICOMTagsOfInterest
     */
    MITKDICOM_EXPORT IDICOMTagsOfInterest* GetTagsOfInterestService();

    /**
     * \brief Extract tag values for paths of interest from DICOM frame information.
     *
     * For each tag path in \p pathsOfInterest, queries the first frame in \p frames
     * for its value and collects the results.
     *
     * \param[in] pathsOfInterest List of DICOMTagPath instances to query.
     * \param[in] frames List of dataset-accessing frame info objects to query from.
     * \return A vector of findings lists, one per path of interest.
     * \sa SetProperties
     */
    MITKDICOM_EXPORT FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest,
      const DICOMDatasetAccessingImageFrameList& frames);

    /**
     * \brief Set DICOM tag values as properties on a BaseData instance.
     *
     * Iterates over all findings and sets each tag value as a TemporoSpatialStringProperty
     * on the given image, using the tag path converted to a property name as the key.
     *
     * \param[in,out] image The BaseData instance on which to set the properties.
     * \param[in] findings The extracted tag findings to set as properties.
     * \sa ExtractPathsOfInterest, DICOMTagPathToPropertyName
     */
    MITKDICOM_EXPORT void SetProperties(BaseData* image, const FindingsListVectorType& findings);
  }
}

#endif
