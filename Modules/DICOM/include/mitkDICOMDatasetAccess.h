/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMDatasetAccess_h
#define mitkDICOMDatasetAccess_h

#include "mitkDICOMTag.h"
#include "mitkDICOMTagPath.h"

#include "MitkDICOMExports.h"

namespace mitk
{
  /**
  Helper class that is used for the result of DICOMDatasetAccess::GetTagValueAsString
   */
  struct MITKDICOM_EXPORT DICOMDatasetFinding
  {
    /**Indicates if value is valid or not.*/
    bool isValid;
    /**The found value.*/
    std::string value;
    /**Tag path of the value*/
    DICOMTagPath path;

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
    typedef std::list<DICOMDatasetFinding> FindingsListType;

    /// \brief Return a filename if possible.
    /// If DICOM is not read from file but from somewhere else (network, database), we might not have files.
    virtual std::string GetFilenameIfAvailable() const = 0;

    /** \brief Return a DICOMDatasetFinding instance of the tag.
    The return containes (if valid) the raw value of the tag as a string.
    \param tag Tag which value should be retreived.
    */
    virtual DICOMDatasetFinding GetTagValueAsString(const DICOMTag& tag) const = 0;

    /** \brief Return a list of DICOMDatasetFindings of the passed tag path.
    The return containes (if valid) the raw value of the tag as a string.
    \param path Tag path which value should be retreived.
    */
    virtual FindingsListType GetTagValueAsString(const DICOMTagPath& path) const = 0;

    virtual ~DICOMDatasetAccess() {};
};


typedef std::vector<DICOMDatasetAccess*> DICOMDatasetList;

}

#endif
