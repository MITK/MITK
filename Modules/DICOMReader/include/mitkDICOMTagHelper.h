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

#ifndef mitkDICOMTagHelper_h
#define mitkDICOMTagHelper_h

#include <unordered_map>

#include <mitkDICOMTag.h>

#include "MitkDICOMReaderExports.h"

namespace mitk
{

    typedef std::unordered_map<const char*, DICOMTag> DICOMTagMapType;

    /** Returns the list of tags that are by default of interest and should
    * be extracted when loading DICOM data. This can for instance be used
    * to with DICOMFileReader::SetAdditionalTagsOfInterest().*/
    DICOMTagMapType MITKDICOMREADER_EXPORT GetCurrentDICOMTagsOfInterest();

    typedef std::unordered_map<std::string, DICOMTag> DefaultDICOMTagMapType;

    /** Returns the list of tags that are by default of interest and should
     * be extracted when loading DICOM data. This can for instance be used
     * to with DICOMFileReader::SetAdditionalTagsOfInterest().*/
    DefaultDICOMTagMapType MITKDICOMREADER_EXPORT GetDefaultDICOMTagsOfInterest();

    /** returns the correct property name for a given DICOMTag instance. */
    std::string MITKDICOMREADER_EXPORT GeneratPropertyNameForDICOMTag(const mitk::DICOMTag& tag);

}

#endif
