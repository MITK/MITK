/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagsOfInterestHelper_h
#define mitkDICOMTagsOfInterestHelper_h

#include <unordered_map>

#include <mitkDICOMTagPath.h>

#include <MitkDICOMExports.h>

namespace mitk
{

    /**
     * \ingroup DICOMModule
     * \brief Map type for DICOM tags of interest.
     *
     * Key is the tag path of interest. Value is an optional user-defined name
     * for the property that should be used to store the tag value(s). An empty
     * value (default) implies using the DICOMTagPath converted to a property name.
     */
    typedef std::map<DICOMTagPath, std::string> DICOMTagPathMapType;

    /**
     * \brief Returns the currently registered DICOM tags of interest.
     *
     * Queries the IDICOMTagsOfInterest service for the tags that have been registered.
     * This list may differ from the default list if additional tags have been added.
     *
     * \return A map of tag paths to optional property names.
     * \sa GetDefaultDICOMTagsOfInterest, IDICOMTagsOfInterest, DICOMFileReader::SetAdditionalTagsOfInterest
     */
    DICOMTagPathMapType MITKDICOM_EXPORT GetCurrentDICOMTagsOfInterest();

    /**
     * \brief Returns the default list of DICOM tags of interest.
     *
     * Returns the built-in list of tags that are by default of interest and should
     * be extracted when loading DICOM data. This can for instance be used
     * with DICOMFileReader::SetAdditionalTagsOfInterest().
     *
     * \return A map of tag paths to optional property names.
     * \sa GetCurrentDICOMTagsOfInterest, DICOMFileReader::SetAdditionalTagsOfInterest
     */
    DICOMTagPathMapType MITKDICOM_EXPORT GetDefaultDICOMTagsOfInterest();
}

#endif
