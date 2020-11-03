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

#include "MitkDICOMExports.h"

namespace mitk
{

    /** Type specifies tags of interest. Key is the tag path of interest.
    * The value is an optional user defined name for the property that should be used to store the tag value(s).
    * Empty value is default and will imply to use the found DICOMTagPath as property name.*/
    typedef std::map<DICOMTagPath, std::string> DICOMTagPathMapType;

    /** Returns the list of tags that are by default of interest and should
    * be extracted when loading DICOM data. This can for instance be used
    * to with DICOMFileReader::SetAdditionalTagsOfInterest().*/
    DICOMTagPathMapType MITKDICOM_EXPORT GetCurrentDICOMTagsOfInterest();

    /** Returns the list of tags that are by default of interest and should
     * be extracted when loading DICOM data. This can for instance be used
     * to with DICOMFileReader::SetAdditionalTagsOfInterest().*/
    DICOMTagPathMapType MITKDICOM_EXPORT GetDefaultDICOMTagsOfInterest();
}

#endif
