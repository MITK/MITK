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

#ifndef mitkDICOMTagsOfInterestHelper_h
#define mitkDICOMTagsOfInterestHelper_h

#include <unordered_map>

#include <mitkDICOMTagPath.h>

#include "MitkDICOMReaderExports.h"

namespace mitk
{

    /** Type specifies tags of interest. Key is the tag path of interest.
    * The value is an optional user defined name for the property that should be used to store the tag value(s).
    * Empty value is default and will imply to use the found DICOMTagPath as property name.*/
    typedef std::map<DICOMTagPath, std::string> DICOMTagPathMapType;

    /** Returns the list of tags that are by default of interest and should
    * be extracted when loading DICOM data. This can for instance be used
    * to with DICOMFileReader::SetAdditionalTagsOfInterest().*/
    DICOMTagPathMapType MITKDICOMREADER_EXPORT GetCurrentDICOMTagsOfInterest();

    /** Returns the list of tags that are by default of interest and should
     * be extracted when loading DICOM data. This can for instance be used
     * to with DICOMFileReader::SetAdditionalTagsOfInterest().*/
    DICOMTagPathMapType MITKDICOMREADER_EXPORT GetDefaultDICOMTagsOfInterest();
}

#endif
