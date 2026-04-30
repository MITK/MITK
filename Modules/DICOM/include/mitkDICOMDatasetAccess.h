/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMDatasetAccess_h
#define mitkDICOMDatasetAccess_h

#include <mitkDICOMTag.h>
#include <mitkDICOMTagPath.h>

#include <MitkDICOMExports.h>

namespace mitk
{
  /**
   * \ingroup DICOMModule
   * \brief Result structure for DICOMDatasetAccess::GetTagValueAsString.
   *
   * Holds the result of a DICOM tag value query, including validity flag,
   * the string value, and the tag path that was queried.
   *
   * \sa DICOMDatasetAccess
   */
  struct MITKDICOM_EXPORT DICOMDatasetFinding
  {
    bool isValid;       ///< Indicates if the finding is valid (tag was found).
    std::string value;  ///< The found tag value as a string.
    DICOMTagPath path;  ///< The tag path of the found value.

    /**
     * \brief Construct a DICOMDatasetFinding.
     * \param[in] valid Whether the finding is valid.
     * \param[in] aValue The tag value string.
     * \param[in] aPath The tag path.
     */
    DICOMDatasetFinding(bool valid = false, const std::string& aValue = "", const DICOMTagPath& aPath = DICOMTagPath()) : isValid(valid), value(aValue), path(aPath)
    {};
  };

/**
  \ingroup DICOMModule
  \brief Interface to datasets that is presented to sorting classes such as DICOMDatasetSorter.

  Minimal interface to hide actual implementation, which might rely on GDCM.
*/
class MITKDICOM_EXPORT DICOMDatasetAccess
{
  public:
    /** \brief A list of DICOMDatasetFinding instances. */
    typedef std::list<DICOMDatasetFinding> FindingsListType;

    /**
     * \brief Return a filename if possible.
     *
     * If DICOM is not read from file but from somewhere else (network, database),
     * we might not have files. In such cases, an empty string may be returned.
     *
     * \return The absolute filename of this dataset, or empty string if unavailable.
     */
    virtual std::string GetFilenameIfAvailable() const = 0;

    /**
     * \brief Return a DICOMDatasetFinding for a single DICOM tag.
     *
     * The return contains (if valid) the raw value of the tag as a string.
     *
     * \param[in] tag The DICOM tag whose value should be retrieved.
     * \return A DICOMDatasetFinding with isValid indicating whether the tag was found.
     */
    virtual DICOMDatasetFinding GetTagValueAsString(const DICOMTag& tag) const = 0;

    /**
     * \brief Return a list of DICOMDatasetFindings for a tag path.
     *
     * The return contains (if valid) the raw values of the matching tags as strings.
     * This overload supports nested/sequence paths.
     *
     * \param[in] path The tag path whose values should be retrieved.
     * \return A list of findings matching the given path.
     */
    virtual FindingsListType GetTagValueAsString(const DICOMTagPath& path) const = 0;

    virtual ~DICOMDatasetAccess() {};
};

/** \brief A list of raw pointers to DICOMDatasetAccess instances. */
typedef std::vector<DICOMDatasetAccess*> DICOMDatasetList;

}

#endif
