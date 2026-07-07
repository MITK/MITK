/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCustomTagParser_h
#define mitkCustomTagParser_h

#include<mitkPropertyList.h>
#include<mitkTemporoSpatialStringProperty.h>

#include <MitkCESTExports.h>

namespace mitk
{
  /**
  \brief Parser for the Siemens private DICOM tag (0x0029, 0x1020) to extract CEST sequence parameters.

  The CustomTagParser can be used to parse the custom DICOM tag of the Siemens private tag
  (0x0029, 0x1020) to extract relevant CEST data and store it as an mitk::PropertyList.

  An initial parsing determines whether the provided string belongs to CEST data at all.
  To make the check and extract the revision number the following rules are applied: \n
  <ol>
    <li>Sequence name (tSequenceFileName) must either
      <ol>
        <li>start with the substring "CEST" (case insensitive), or</li>
        <li>contain the substring "_CEST" (case insensitive).</li>
      </ol>
    </li>
    <li>Sequence name (tSequenceFileName) must contain the substring "_Rev" (case insensitive).</li>
    <li>All numbers after "_Rev" represent the revision number; until either
      <ol>
        <li>the next _, or</li>
        <li>end of sequence name.</li>
      </ol>
    </li>
  </ol>

  Which custom parameters to save and to which property name can be controlled by a JSON file.
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
    <li> REVISIONNUMBER is the revision number of this JSON parameter mapping (files with non-digit characters in their
  name will be ignored)
    <li> sWiPMemBlock.alFree[1] is the name of one parameter in the private DICOM tag
    <li> AdvancedMode is the name of the property the content of sWiPMemBlock.alFree[1] should be saved to
  </ul>

  \note It is assumed that the entire content of tag (0x0029, 0x1020) is provided and that it is hex encoded
  (12\23\04...).

  If the sampling type is list, the parser will try to access LIST.txt at the location provided in the constructor to
  read the offsets. For T1 sequences, TREC.txt will be read from the same location.

  \sa CESTImageNormalizationFilter, CESTPropertyHelper, ExtractCESTOffset
  */
  class MITKCEST_EXPORT CustomTagParser
  {
  public:
    /**
    \brief Construct a CustomTagParser for DICOM files at the given path.

    \param[in] relevantFile Path to one of the DICOM files to be loaded, or the directory
               containing the DICOM files. The directory is used to locate LIST.txt or TREC.txt
               files for offset/T1 recovery time data.
    */
    CustomTagParser(std::string relevantFile);

    /**
    \brief Parse a DICOM TemporoSpatialStringProperty and return extracted CEST parameters.

    Extracts the string value from the provided property and delegates to ParseDicomPropertyString().

    \param[in] dicomProperty Pointer to the TemporoSpatialStringProperty containing the hex-encoded
               Siemens private tag content. If nullptr, an error is logged and an empty PropertyList is returned.
    \return A PropertyList containing the extracted CEST parameters with the "CEST." prefix.
    \sa ParseDicomPropertyString
    */
    mitk::PropertyList::Pointer ParseDicomProperty(mitk::TemporoSpatialStringProperty *dicomProperty);

    /**
    \brief Parse a hex-encoded DICOM tag string and return extracted CEST parameters.

    Decodes the hex-encoded string, extracts the ASCCONV parameter block, determines the sequence
    revision, loads the appropriate JSON parameter mapping, and populates a PropertyList with
    all matched CEST parameters. For CEST/WASABI sequences, frequency offsets are computed or
    loaded from LIST.txt. For T1 sequences, recovery times are loaded from TREC.txt.

    \param[in] dicomPropertyString The hex-encoded content of the Siemens private DICOM tag
               (0x0029, 0x1020). An empty string results in an empty PropertyList.
    \return A PropertyList containing the extracted CEST parameters with the "CEST." prefix.
               Properties are also registered with the IPropertyPersistence service.
    \sa ParseDicomProperty
    */
    mitk::PropertyList::Pointer ParseDicomPropertyString(std::string dicomPropertyString);

    /**
    \brief Read a space-separated list of values from a text file.

    Reads all lines from the file at the given path and concatenates them into a single
    space-separated string. Used internally to load LIST.txt (offsets) and TREC.txt (T1 recovery times).

    \param[in] filePath Absolute path to the text file to read.
    \return A string containing all values from the file separated by spaces.
            Returns an empty string if the file cannot be opened.
    */
    static std::string ReadListFromFile(const std::string& filePath);

    /**
    \brief Extract the revision number from a CEST sequence file name.

    Applies the naming rules described in the class documentation to identify and extract
    the numeric revision substring from the sequence file name.

    \param[in] sequenceFileName The CEST sequence file name (e.g., "CEST_Rev1416_someSequence").
    \return The revision number as a string, or an empty string if the file name is valid
            but contains no revision number after "_Rev".
    \throw mitk::Exception if the file name does not contain a valid CEST prefix ("CEST" or "_CEST").
    \throw mitk::Exception if the file name contains a CEST prefix but no "_Rev" substring.
    */
    static std::string ExtractRevision(std::string sequenceFileName);

    /**
    \brief Set the strategy used to determine the type of CEST data.

    \param[in] parseStrategy The parse strategy to use. Valid values are:
               - "Automatic" (default): auto-detect whether data is T1 or CEST/WASABI
               - "T1": force interpretation as T1 sequence
               - "CEST/WASABI": force interpretation as CEST or WASABI sequence
    */
    void SetParseStrategy(std::string parseStrategy);

    /**
    \brief Set the strategy for matching revision numbers to JSON parameter mappings.

    \param[in] revisionMappingStrategy The mapping strategy to use. Valid values are:
               - "Fuzzy" (default): use the closest lower or equal revision mapping
               - "Strict": require an exact revision match; throws if none is found
    */
    void SetRevisionMappingStrategy(std::string revisionMappingStrategy);

    /** \brief Name of the property for the data acquisition revision ("CEST.Revision"). */
    static const std::string m_RevisionPropertyName;

    /** \brief Name of the property for the JSON parameter mapping revision ("CEST.revision_json"). */
    static const std::string m_JSONRevisionPropertyName;

    /** \brief Prefix for all CEST-related property names ("CEST."). */
    static const std::string m_CESTPropertyPrefix;

  protected:
    std::string GetRevisionAppropriateJSONString(std::string revisionString);
    void GetClosestLowerRevision(std::string revisionString);
    std::string GetClosestLowerRevision(std::string revisionString, std::vector<int> availableRevisionsVector);

    /// Decides whether or not the image is likely to be a T1Map, if not it is assumed to be a CEST sequence
    bool IsT1Sequence(std::string preparationType, std::string recoveryMode, std::string spoilingType, std::string revisionString);

    /// Get a string filled with the properly formatted offsets based on the sampling type and offset
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

}

#endif
