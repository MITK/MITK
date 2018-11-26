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
  To make the check and extract the revision number the following rules are aplied: \n
  <ol>
    <li>Sequence name (tSequenceFileName) must either
      <ol>
        <li>start with the substring "CEST" (case insensitiv), or</li>
        <li>contain the substring "_CEST" (case insensitiv).</li>
      </ol>
    </li>
    <li>Sequence name (tSequenceFileName) must contain the substring "_Rev" (case insensitiv).</li>
    <li>All numbers after "_Rev" represent the revision number; until either
      <ol>
        <li>the next _, or</li>
        <li>end of sequence name.</li>
      </ol>
    </li>
  </ol>

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

    /** Extract the revision out of the passed sequenceFileName. If the file name is not a valid CEST file name
      (see rules in the class documentation) exceptions will be thrown. If the file name is valid but contains no
      revision number an empty string will be returned.
    */
    static std::string ExtractRevision(std::string sequenceFileName);

    void SetParseStrategy(std::string parseStrategy);
    void SetRevisionMappingStrategy(std::string revisionMappingStrategy);

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
    
    /// Decides whether or not the image is likely to be a T1Map, if not it is assumed to be a CEST sequence
    bool IsT1Sequence(std::string preparationType, std::string recoveryMode, std::string spoilingType, std::string revisionString);

    /// Get a string filled with the properly formated offsets based on the sampling type and offset
    std::string GetOffsetString(std::string samplingType, std::string offset, std::string measurements);

    /// returns a vector revision numbers of all REVISIONNUMBER.json found beside the MitkCEST library
    std::vector<int> GetExternalRevisions();
    /// returns a vector revision numbers of all REVISIONNUMBER.json provided as resources during the compile
    std::vector<int> GetInternalRevisions();

    /// returns the path where external jsons are expected to be located
    std::string GetExternalJSONDirectory();

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
    /// Should the kind of data be automatically determined or should it be parsed as a specific one
    std::string m_ParseStrategy;
    /// How to handle parameter mapping based on absent revision jsons
    std::string m_RevisionMappingStrategy;
  };

  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_TOTALSCANTIME();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_PREPERATIONTYPE();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_RECOVERYMODE();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_SPOILINGTYPE();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_OFFSETS();
  const std::string MITKCEST_EXPORT CEST_PROPERTY_NAME_TREC();

}

#endif // MITKCUSTOMTAGPARSER_H
