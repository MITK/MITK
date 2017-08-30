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

#ifndef MITKCUSTOMTAGPARSER_H
#define MITKCUSTOMTAGPARSER_H

#include<mitkPropertyList.h>
#include<mitkTemporoSpatialStringProperty.h>

#include <MitkCESTExports.h>

namespace mitk
{
  /**
  The custom tag parser can be used to parse the custom dicom tag of the siemens private tag
  (0x0029, 0x1020) to extract relevant CEST data.

  An initial parsing determines whether the provided string belongs to CEST data at all.
  If the "tSequenceFileName" is of the format "{WHATEVER}CEST_Rev####" it is assumed that the
  data is indeed CEST data and was taken with revision #### (not limited to four digits).

  Which custom parameters to save and to which property name can be controlled by a json file.
  This file can be either provided as a resource for the MitkCEST module during compilation or
  placed next to the MitkCEST library in your binary folder.

  The expected format for the file "REVISIONNUMBER.json": <br>
  { <br>
  "REVISIONNUMBER" : "revision_json", <br>
  "sWiPMemBlock.alFree[1]" : "AdvancedMode", <br>
  "sWiPMemBlock.alFree[2]" : "RetreatMode" <br>
  } <br>

  where :
  <ul>
    <li> REVISIONNUMBER is the revision number of this json parameter mapping (files with non digit characters in their
  name will be ignored)
    <li> sWiPMemBlock.alFree[1] is the name of one parameter in the private dicom tag
    <li> AdvancedMode is the name of the property the content of sWiPMemBlock.alFree[1] should be saved to
  </ul>

  \note It is assumed that the entire content of tag (0x0029, 0x1020) is provided and that it es hex encoded
  (12\23\04...).

  If the sampling type is list it will try to access LIST.txt at the location provided in the constructor to
  read the offsets.
  */
  class MITKCEST_EXPORT CustomTagParser
  {
  public:
    /// the constructor expects a path to one of the files to be loaded or the directory of the dicom files
    CustomTagParser(std::string relevantFile);

    /// parse the provided dicom property and return a property list based on the closest revision parameter mapping
    mitk::PropertyList::Pointer ParseDicomProperty(mitk::TemporoSpatialStringProperty *dicomProperty);
    /// parse the provided string and return a property list based on the closest revision parameter mapping
    mitk::PropertyList::Pointer ParseDicomPropertyString(std::string dicomPropertyString);

    /// name of the property for the offsets, including normalization offsets
    static const std::string m_OffsetsPropertyName;

    /// name of the property for the data acquisition revision
    static const std::string m_RevisionPropertyName;

    /// name of the property for the json parameter mapping revision
    static const std::string m_JSONRevisionPropertyName;

    /// prefix for all CEST related property names
    static const std::string m_CESTPropertyPrefix;

  protected:
    std::string GetRevisionAppropriateJSONString(std::string revisionString);
    void GetClosestLowerRevision(std::string revisionString);
    std::string GetClosestLowerRevision(std::string revisionString, std::vector<int> availableRevisionsVector);

    /// Get a string filled with the properly formated offsets based on the sampling type and offset
    std::string GetOffsetString(std::string samplingType, std::string offset, std::string measurements);

    /// returns a vector revision numbers of all REVISIONNUMBER.json found beside the MitkCEST library
    std::vector<int> GetExternalRevisions();
    /// returns a vector revision numbers of all REVISIONNUMBER.json provided as resources during the compile
    std::vector<int> GetInternalRevisions();

    /// the closest lower revision provided as resource, empty if none found
    std::string m_ClosestInternalRevision;
    /// the closest lower revision provided as a json beside the library, empty if none found
    std::string m_ClosestExternalRevision;

    /// revision independent mapping to inject into the revision dependent json string
    static const std::string m_RevisionIndependentMapping;
    /// default revision dependent json string if none is found
    static const std::string m_DefaultJsonString;
    /// path to the dicom data
    std::string m_DicomDataPath;
  };
}

#endif // MITKCUSTOMTAGPARSER_H
