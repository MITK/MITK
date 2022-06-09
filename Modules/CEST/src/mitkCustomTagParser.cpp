/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCustomTagParser.h"

#include <mitkProperties.h>
#include <mitkStringProperty.h>

#include "mitkCESTPropertyHelper.h"
#include "mitkIPropertyPersistence.h"

#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include <itksys/SystemTools.hxx>

#include <Poco/Glob.h>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using namespace nlohmann;

namespace
{
  mitk::IPropertyPersistence *GetPersistenceService()
  {
    mitk::IPropertyPersistence *result = nullptr;

    std::vector<us::ServiceReference<mitk::IPropertyPersistence>> persRegisters =
      us::GetModuleContext()->GetServiceReferences<mitk::IPropertyPersistence>();
    if (!persRegisters.empty())
    {
      if (persRegisters.size() > 1)
      {
        MITK_WARN << "Multiple property description services found. Using just one.";
      }
      result = us::GetModuleContext()->GetService<mitk::IPropertyPersistence>(persRegisters.front());
    }

    return result;
  };
}

const std::string mitk::CustomTagParser::m_CESTPropertyPrefix = "CEST.";
const std::string mitk::CustomTagParser::m_RevisionPropertyName = m_CESTPropertyPrefix + "Revision";
const std::string mitk::CustomTagParser::m_JSONRevisionPropertyName = m_CESTPropertyPrefix + "revision_json";

const std::string mitk::CustomTagParser::m_RevisionIndependentMapping = R"(
  "sProtConsistencyInfo.tSystemType" : "SysType",
  "sProtConsistencyInfo.flNominalB0" : "NominalB0",
  "sTXSPEC.asNucleusInfo[0].lFrequency" : "FREQ",
  "sTXSPEC.asNucleusInfo[0].flReferenceAmplitude" : "RefAmp",
  "alTR[0]" : "TR",
  "alTE[0]" : "TE",
  "lAverages" : "averages",
  "lRepetitions" : "repetitions",
  "adFlipAngleDegree[0]" : "ImageFlipAngle",
  "lTotalScanTimeSec" : "TotalScanTime",
)";
const std::string mitk::CustomTagParser::m_DefaultJsonString = R"({
  "default mapping, corresponds to revision 1416" : "revision_json",
  "sWiPMemBlock.alFree[1]" : "AdvancedMode",
  "sWiPMemBlock.alFree[2]" : "RecoveryMode",
  "sWiPMemBlock.alFree[3]" : "DoubleIrrMode",
  "sWiPMemBlock.alFree[4]" : "BinomMode",
  "sWiPMemBlock.alFree[5]" : "MtMode",
  "sWiPMemBlock.alFree[6]" : "PreparationType",
  "sWiPMemBlock.alFree[7]" : "PulseType",
  "sWiPMemBlock.alFree[8]" : "SamplingType",
  "sWiPMemBlock.alFree[9]" : "SpoilingType",
  "sWiPMemBlock.alFree[10]" : "measurements",
  "sWiPMemBlock.alFree[11]" : "NumberOfPulses",
  "sWiPMemBlock.alFree[12]" : "NumberOfLockingPulses",
  "sWiPMemBlock.alFree[13]" : "PulseDuration",
  "sWiPMemBlock.alFree[14]" : "DutyCycle",
  "sWiPMemBlock.alFree[15]" : "RecoveryTime",
  "sWiPMemBlock.alFree[16]" : "RecoveryTimeM0",
  "sWiPMemBlock.alFree[17]" : "ReadoutDelay",
  "sWiPMemBlock.alFree[18]" : "BinomDuration",
  "sWiPMemBlock.alFree[19]" : "BinomDistance",
  "sWiPMemBlock.alFree[20]" : "BinomNumberofPulses",
  "sWiPMemBlock.alFree[21]" : "BinomPreRepetions",
  "sWiPMemBlock.alFree[22]" : "BinomType",
  "sWiPMemBlock.adFree[1]" : "Offset",
  "sWiPMemBlock.adFree[2]" : "B1Amplitude",
  "sWiPMemBlock.adFree[3]" : "AdiabaticPulseMu",
  "sWiPMemBlock.adFree[4]" : "AdiabaticPulseBW",
  "sWiPMemBlock.adFree[5]" : "AdiabaticPulseLength",
  "sWiPMemBlock.adFree[6]" : "AdiabaticPulseAmp",
  "sWiPMemBlock.adFree[7]" : "FermiSlope",
  "sWiPMemBlock.adFree[8]" : "FermiFWHM",
  "sWiPMemBlock.adFree[9]" : "DoubleIrrDuration",
  "sWiPMemBlock.adFree[10]" : "DoubleIrrAmplitude",
  "sWiPMemBlock.adFree[11]" : "DoubleIrrRepetitions",
  "sWiPMemBlock.adFree[12]" : "DoubleIrrPreRepetitions"
})";

mitk::CustomTagParser::CustomTagParser(std::string relevantFile) : m_ClosestInternalRevision(""), m_ClosestExternalRevision("")
{
  std::string pathToDirectory;
  std::string fileName;
  itksys::SystemTools::SplitProgramPath(relevantFile, pathToDirectory, fileName);
  m_DicomDataPath = pathToDirectory;
  m_ParseStrategy = "Automatic";
  m_RevisionMappingStrategy = "Fuzzy";
}

std::string mitk::CustomTagParser::ExtractRevision(std::string sequenceFileName)
{
  //all rules are case insesitive. Thus we convert everything to lower case
  //in order to check everything only once.
  std::string cestPrefix = "cest";
  std::string cestPrefix2 = "_cest";
  std::string cestPrefix3 = "\\cest"; //this version covers the fact that the strings extracted
                                      //from the SIEMENS tag has an additional prefix that is seperated by backslash.
  std::string revisionPrefix = "_rev";
  std::transform(sequenceFileName.begin(), sequenceFileName.end(), sequenceFileName.begin(), ::tolower);

  bool isCEST = sequenceFileName.compare(0, cestPrefix.length(), cestPrefix) == 0;
  std::size_t foundPosition = 0;

  if (!isCEST)
  {
    foundPosition = sequenceFileName.find(cestPrefix2);
    isCEST = foundPosition != std::string::npos;
  }

  if (!isCEST)
  {
    foundPosition = sequenceFileName.find(cestPrefix3);
    isCEST = foundPosition != std::string::npos;
  }

  if (!isCEST)
  {
    mitkThrow() << "Invalid CEST sequence file name. No CEST prefix found. Could not extract revision.";
  }

  foundPosition = sequenceFileName.find(revisionPrefix, foundPosition);
  if (foundPosition == std::string::npos)
  {
    mitkThrow() << "Invalid CEST sequence file name. No revision prefix was found in CEST sequence file name. Could not extract revision.";
  }

  std::string revisionString = sequenceFileName.substr(foundPosition + revisionPrefix.length(), std::string::npos);
  std::size_t firstNoneNumber = revisionString.find_first_not_of("0123456789");
  if (firstNoneNumber != std::string::npos)
  {
    revisionString.erase(firstNoneNumber, std::string::npos);
  }

  return revisionString;
}

bool mitk::CustomTagParser::IsT1Sequence(std::string preparationType,
  std::string recoveryMode,
  std::string spoilingType,
  std::string revisionString)
{
  bool isT1 = false;

  // if a forced parse strategy is set, use that one
  if ("T1" == m_ParseStrategy)
  {
    return true;
  }
  if ("CEST/WASABI" == m_ParseStrategy)
  {
    return false;
  }

  if (("T1Recovery" == preparationType) || ("T1Inversion" == preparationType))
  {
    isT1 = true;
  }

  // How to interpret the recoveryMode depends on the age of the sequence
  // older sequences use 0 = false and 1 = true, newer ones 1 = false and 2 = true.
  // A rough rule of thumb is to assume that if the SpoilingType is 0, then the first
  // convention is chosen, if it is 1, then the second applies. Otherwise
  // we assume revision 1485 and newer to follow the new convention.
  // This unfortunate heuristic is due to somewhat arbitrary CEST sequence implementations.
  if (!isT1)
  {
    std::string thisIsTrue = "1";
    std::string thisIsFalse = "0";
    if ("0" == spoilingType)
    {
      thisIsFalse = "0";
      thisIsTrue = "1";
    }
    else if ("1" == spoilingType)
    {
      thisIsFalse = "1";
      thisIsTrue = "2";
    }
    else
    {
      int revisionNrWeAssumeToBeDifferenciating = 1485;
      if (std::stoi(revisionString) - revisionNrWeAssumeToBeDifferenciating < 0)
      {
        thisIsFalse = "0";
        thisIsTrue = "1";
      }
      else
      {
        thisIsFalse = "1";
        thisIsTrue = "2";
      }
    }

    if (thisIsFalse == recoveryMode)
    {
      isT1 = false;
    }
    else if (thisIsTrue == recoveryMode)
    {
      isT1 = true;
    }

  }

  return isT1;
}

mitk::PropertyList::Pointer mitk::CustomTagParser::ParseDicomPropertyString(std::string dicomPropertyString)
{
  auto results = mitk::PropertyList::New();
  if ("" == dicomPropertyString)
  {
    //MITK_ERROR << "Could not parse empty custom dicom string";
    return results;
  }

  auto comp = [](const std::string& s1, const std::string& s2)
  {
    return boost::algorithm::lexicographical_compare(s1, s2, boost::algorithm::is_iless());
  };

  std::map<std::string, std::string, decltype(comp)> privateParameters(comp);

  // The Siemens private tag contains information like "43\52\23\34".
  // We jump over each "\" and convert the number;
  std::string bytes;

  {
    const std::size_t SUBSTR_LENGTH = 2;
    const std::size_t INPUT_LENGTH = dicomPropertyString.length();

    if (INPUT_LENGTH < SUBSTR_LENGTH)
      return results;

    const std::size_t MAX_INPUT_OFFSET = INPUT_LENGTH - SUBSTR_LENGTH;
    bytes.reserve(INPUT_LENGTH / 3 + 1);

    try
    {
      for (std::size_t i = 0; i <= MAX_INPUT_OFFSET; i += 3)
      {
        std::string byte_string = dicomPropertyString.substr(i, SUBSTR_LENGTH);
        int byte = static_cast<std::string::value_type>(std::stoi(byte_string.c_str(), nullptr, 16));
        bytes.push_back(byte);
      }
    }
    catch (const std::invalid_argument&) // std::stoi() could not perform conversion
    {
      return results;
    }
  }

  // extract parameter list
  std::string parameterListString;

  {
    const std::string ASCCONV_MARKER = "###";
    const std::string ASCCONV_BEGIN = "### ASCCONV BEGIN";
    const std::string ASCCONV_END = "### ASCCONV END";

    auto ascconvBeginPos = bytes.find(ASCCONV_BEGIN);
    if (std::string::npos == ascconvBeginPos)
      return results;
    ascconvBeginPos += ASCCONV_BEGIN.length();

    ascconvBeginPos = bytes.find(ASCCONV_MARKER, ascconvBeginPos);
    if (std::string::npos == ascconvBeginPos)
      return results;
    ascconvBeginPos += ASCCONV_MARKER.length(); // closing "###"

    auto ascconvEndPos = bytes.find(ASCCONV_END, ascconvBeginPos);
    if (std::string::npos == ascconvEndPos)
      return results;

    auto count = ascconvEndPos - ascconvBeginPos;

    parameterListString = bytes.substr(ascconvBeginPos, count);
  }

  boost::replace_all(parameterListString, "\r\n", "\n");
  boost::replace_all(parameterListString, "\t", "");
  boost::char_separator<char> newlineSeparator("\n");
  boost::tokenizer<boost::char_separator<char>> parameters(parameterListString, newlineSeparator);
  for (const auto &parameter : parameters)
  {
    std::vector<std::string> parts;
    boost::split(parts, parameter, boost::is_any_of("="));

    if (parts.size() == 2)
    {
      parts[0].erase(std::remove(parts[0].begin(), parts[0].end(), ' '), parts[0].end());
      parts[1].erase(parts[1].begin(), parts[1].begin() + 1); // first character is a space
      privateParameters[parts[0]] = parts[1];
    }
  }

  std::string revisionString = "";

  try
  {
    revisionString = ExtractRevision(privateParameters["tSequenceFileName"]);
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Cannot deduce revision information. Reason: "<< e.what();
    return results;
  }

  results->SetProperty(m_RevisionPropertyName, mitk::StringProperty::New(revisionString));

  std::string jsonString = GetRevisionAppropriateJSONString(revisionString);

  json root;
  try
  {
    root = json::parse(jsonString);
  }
  catch (const json::exception &e)
  {
    mitkThrow() << "Could not parse json file. Error was:\n" << e.what();
  }

  for (const auto &it : root.items())
  {
    if (it.value().is_string())
    {
      auto propertyName = m_CESTPropertyPrefix + it.value().get<std::string>();
      if (m_JSONRevisionPropertyName == propertyName)
      {
        results->SetProperty(propertyName, mitk::StringProperty::New(it.key()));
      }
      else
      {
        results->SetProperty(propertyName, mitk::StringProperty::New(privateParameters[it.key()]));
      }
    }
    else
    {
      MITK_ERROR << "Currently no support for nested dicom tag descriptors in json file.";
    }
  }

  std::string offset = "";
  std::string measurements = "";
  results->GetStringProperty("CEST.Offset", offset);
  results->GetStringProperty("CEST.measurements", measurements);

  if (measurements.empty())
  {
    std::string stringRepetitions = "";
    results->GetStringProperty("CEST.repetitions", stringRepetitions);

    std::string stringAverages = "";
    results->GetStringProperty("CEST.averages", stringAverages);

    const auto ERROR_STRING = "Could not find measurements, fallback assumption of repetitions + averages could not be determined either.";

    if (!stringRepetitions.empty() && !stringAverages.empty())
    {
      std::stringstream measurementStream;

      try
      {
        measurementStream << std::stoi(stringRepetitions) + std::stoi(stringAverages);
        measurements = measurementStream.str();
        MITK_INFO << "Could not find measurements, assuming repetitions + averages. That is: " << measurements;
      }
      catch (const std::invalid_argument&)
      {
        MITK_ERROR << ERROR_STRING;
      }
    }
    else
    {
      MITK_WARN << ERROR_STRING;
    }
  }

  std::string preparationType = "";
  std::string recoveryMode = "";
  std::string spoilingType = "";
  results->GetStringProperty(CEST_PROPERTY_NAME_PREPERATIONTYPE().c_str(), preparationType);
  results->GetStringProperty(CEST_PROPERTY_NAME_RECOVERYMODE().c_str(), recoveryMode);
  results->GetStringProperty(CEST_PROPERTY_NAME_SPOILINGTYPE().c_str(), spoilingType);

  if (this->IsT1Sequence(preparationType, recoveryMode, spoilingType, revisionString))
  {
    MITK_INFO << "Parsed as T1 image";

    std::stringstream trecStream;

    std::string trecPath = m_DicomDataPath + "/TREC.txt";
    auto trec = ReadListFromFile(trecPath);

    if(trec.empty())
    {
      MITK_WARN << "Assumed T1, but could not load TREC at " << trecPath;
    }

    results->SetStringProperty(CEST_PROPERTY_NAME_TREC().c_str(), trec.c_str());
  }
  else
  {
    MITK_INFO << "Parsed as CEST or WASABI image";
    std::string sampling = "";
    bool hasSamplingInformation = results->GetStringProperty("CEST.SamplingType", sampling);
    if (hasSamplingInformation)
    {
      std::string offsets = GetOffsetString(sampling, offset, measurements);
      results->SetStringProperty(CEST_PROPERTY_NAME_OFFSETS().c_str(), offsets.c_str());
    }
    else
    {
      MITK_WARN << "Could not determine sampling type.";
    }
  }


  //persist all properties
  mitk::IPropertyPersistence *persSrv = GetPersistenceService();
  if (persSrv)
  {
    auto propertyMap = results->GetMap();
    for (auto const &prop : *propertyMap)
    {
      PropertyPersistenceInfo::Pointer info = PropertyPersistenceInfo::New();
      std::string key = prop.first;
      std::replace(key.begin(), key.end(), '.', '_');
      info->SetNameAndKey(prop.first, key);

      persSrv->AddInfo(info);
    }
  }

  return results;
}

std::string mitk::CustomTagParser::ReadListFromFile(const std::string& filePath)
{
  std::stringstream listStream;
  std::ifstream list(filePath.c_str());
  list.imbue(std::locale("C"));

  if (list.good())
  {
    std::string currentValue;
    while (std::getline(list, currentValue))
    {
      listStream << currentValue << " ";
    }
  }
  return listStream.str();
}

mitk::PropertyList::Pointer mitk::CustomTagParser::ParseDicomProperty(mitk::TemporoSpatialStringProperty *dicomProperty)
{
  if (!dicomProperty)
  {
    MITK_ERROR << "DICOM property empty";
  }

  auto results = mitk::PropertyList::New();

  if (dicomProperty)
  {
    results = ParseDicomPropertyString(dicomProperty->GetValue());
  }

  return results;
}

std::vector<int> mitk::CustomTagParser::GetInternalRevisions()
{
  const std::vector<us::ModuleResource> configs =
    us::GetModuleContext()->GetModule()->FindResources("/", "*.json", false);

  std::vector<int> availableRevisionsVector;

  for (const auto& resource : configs)
  {
    availableRevisionsVector.push_back(std::stoi(resource.GetBaseName()));
  }

  return availableRevisionsVector;
}

std::vector<int> mitk::CustomTagParser::GetExternalRevisions()
{
  std::string stringToJSONDirectory = GetExternalJSONDirectory();

  std::string prospectiveJsonsPath = stringToJSONDirectory + "/*.json";

  std::set<std::string> JsonFiles;
  Poco::Glob::glob(prospectiveJsonsPath, JsonFiles, Poco::Glob::GLOB_CASELESS);

  std::vector<int> availableRevisionsVector;

  for (const auto& jsonpath : JsonFiles)
  {
    std::string jsonDir;
    std::string jsonName;
    itksys::SystemTools::SplitProgramPath(jsonpath, jsonDir, jsonName);
    std::string revision = itksys::SystemTools::GetFilenameWithoutExtension(jsonName);

    // disregard jsons which contain letters in their name
    bool onlyNumbers = (revision.find_first_not_of("0123456789") == std::string::npos);

    if(onlyNumbers)
    {
      availableRevisionsVector.push_back(std::stoi(revision));
    }
  }

  return availableRevisionsVector;
}

std::string mitk::CustomTagParser::GetClosestLowerRevision(std::string revisionString, std::vector<int> availableRevisionsVector)
{

  // descending order
  std::sort(availableRevisionsVector.begin(), availableRevisionsVector.end(), std::greater<>());

  int revision = std::stoi(revisionString);

  int index = 0;
  int numberOfRevisions = availableRevisionsVector.size();

  while (index < numberOfRevisions)
  {
    // current mapping still has a higher revision number
    if ((availableRevisionsVector[index] - revision) > 0)
    {
      ++index;
    }
    else
    {
      break;
    }
  }

  if (index < numberOfRevisions)
  {
    std::stringstream foundRevisionStream;
    foundRevisionStream << availableRevisionsVector[index];

    return foundRevisionStream.str();
  }

  return "";
}

void mitk::CustomTagParser::GetClosestLowerRevision(std::string revisionString)
{
  m_ClosestInternalRevision = GetClosestLowerRevision(revisionString, GetInternalRevisions());
  m_ClosestExternalRevision = GetClosestLowerRevision(revisionString, GetExternalRevisions());

  if ("Strict" == m_RevisionMappingStrategy && !((0 == m_ClosestInternalRevision.compare(revisionString)) ||
                                                 (0 == m_ClosestExternalRevision.compare(revisionString))))
  { // strict revision mapping and neither revision does match the dicom meta data
    std::stringstream errorMessageStream;
    errorMessageStream << "\nCould not parse dicom data in strict mode, data revision " << revisionString
      << " has no known matching parameter mapping. To use the closest known older parameter mapping select the "
      << "\"Fuzzy\" revision mapping option when loading the data.\n"
      << "\nCurrently known revision mappings are:\n  Precompiled:";
    for (const auto revision : GetInternalRevisions())
    {
      errorMessageStream << " " << revision;
    }
    errorMessageStream << "\n  External:";
    for (const auto revision : GetExternalRevisions())
    {
      errorMessageStream << " " << revision;
    }
    errorMessageStream << "\n\nExternal revision mapping descriptions should be located at\n\n";
    std::string stringToJSONDirectory = GetExternalJSONDirectory();
    errorMessageStream << stringToJSONDirectory;

    errorMessageStream << "\n\nTo provide an external mapping for this revision create a " << revisionString
                       << ".json there. You might need to create the directory first.";

    mitkThrow() << errorMessageStream.str();
  }
}

std::string mitk::CustomTagParser::GetRevisionAppropriateJSONString(std::string revisionString)
{
  std::string returnValue = "";

  if ("" == revisionString)
  {
    MITK_WARN << "Could not extract revision";
  }
  else
  {
    GetClosestLowerRevision(revisionString);

    bool useExternal = false;
    bool useInternal = false;

    if ("" != m_ClosestExternalRevision)
    {
      useExternal = true;
    }
    if ("" != m_ClosestInternalRevision)
    {
      useInternal = true;
    }

    if (useExternal && useInternal)
    {
      if (std::stoi(m_ClosestInternalRevision) > std::stoi(m_ClosestExternalRevision))
      {
        useExternal = false;
      }
    }

    if (useExternal)
    {
      std::string stringToJSONDirectory = GetExternalJSONDirectory();

      std::string prospectiveJsonPath = stringToJSONDirectory + "/" + m_ClosestExternalRevision + ".json";

      std::ifstream externalJSON(prospectiveJsonPath.c_str());

      if (externalJSON.good())
      {
        MITK_INFO << "Found external json for CEST parameters at " << prospectiveJsonPath;

        std::stringstream buffer;
        buffer << externalJSON.rdbuf();

        returnValue = buffer.str();

        useInternal = false;
      }
    }

    if (useInternal)
    {
      std::string filename = m_ClosestInternalRevision + ".json";
      us::ModuleResource jsonResource = us::GetModuleContext()->GetModule()->GetResource(filename);

      if (jsonResource.IsValid() && jsonResource.IsFile())
      {
        MITK_INFO << "Found no external json for CEST parameters. Closest internal mapping is for revision "
                  << m_ClosestInternalRevision;
        us::ModuleResourceStream jsonStream(jsonResource);
        std::stringstream buffer;
        buffer << jsonStream.rdbuf();
        returnValue = buffer.str();
      }
    }
  }

  if ("" == returnValue)
  {
    MITK_WARN << "Could not identify parameter mapping for the given revision " << revisionString
              << ", using default mapping.";
    returnValue = m_DefaultJsonString;
  }

  // inject the revision independent mapping before the first newline
  {
    returnValue.insert(returnValue.find("\n"), m_RevisionIndependentMapping);
  }

  return returnValue;
}

std::string mitk::CustomTagParser::GetOffsetString(std::string samplingType, std::string offset, std::string measurements)
{
  std::stringstream results;
  results.imbue(std::locale("C"));

  std::string normalizationIndicatingOffset = "-300";

  double offsetDouble = 0.0;
  int measurementsInt = 0;

  bool validOffset = false;
  bool validMeasurements = false;

  if ("" != offset)
  {
    validOffset = true;
    offsetDouble = std::stod(offset);
  }
  if ("" != measurements)
  {
    validMeasurements = true;
    measurementsInt = std::stoi(measurements);
  }

  std::vector<double> offsetVector;

  if (validOffset && validMeasurements)
  {
    for (int step = 0; step < measurementsInt -1; ++step)
    {
      double currentOffset = -offsetDouble + 2 * step * offsetDouble / (measurementsInt - 2.0);
      offsetVector.push_back(currentOffset);
    }
  }
  else
  {
    MITK_WARN << "Invalid offset or measurements, offset calculation will only work for list sampling type.";
  }

  if (samplingType == "1" || samplingType == "Regular")
  {
    if (validOffset && validMeasurements)
    {
      results << normalizationIndicatingOffset << " ";
      for (const auto& entry : offsetVector)
      {
        results << entry << " ";
      }
    }
  }
  else if (samplingType == "2" || samplingType == "Alternating")
  {
    if (validOffset && validMeasurements)
    {
      results << normalizationIndicatingOffset << " ";
      for (auto& entry : offsetVector)
      {
        entry = std::abs(entry);
      }

      std::sort(offsetVector.begin(), offsetVector.end(), std::greater<>());

      for (unsigned int index = 0; index < offsetVector.size(); ++index)
      {
        offsetVector[index] = std::pow(-1, index) * offsetVector[index];
      }

      for (auto& entry : offsetVector)
      {
        results << entry << " ";
      }
    }
  }
  else if (samplingType == "3" || samplingType == "List")
  {
    std::string listPath = m_DicomDataPath + "/LIST.txt";

    auto values = ReadListFromFile(listPath);

    if (!values.empty())
    {
      results << values;
    }
    else
    {
      MITK_ERROR << "Could not load list at " << listPath;
    }
  }
  else if (samplingType == "4" || samplingType == "SingleOffset")
  {
    if (validOffset && validMeasurements)
    {
      results << normalizationIndicatingOffset << " ";
      for (int step = 0; step < measurementsInt - 1; ++step)
      {
        results << offsetDouble << " ";
      }
    }
  }
  else
  {
    MITK_WARN << "Encountered unknown sampling type.";
  }

  std::string resultString = results.str();
  // replace multiple spaces by a single space
  std::string::iterator newEnditerator =
    std::unique(resultString.begin(), resultString.end(),
      [=](char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }
  );
  resultString.erase(newEnditerator, resultString.end());

  if ((resultString.length() > 0) && (resultString.at(resultString.length() - 1) == ' '))
  {
    resultString.erase(resultString.end() - 1, resultString.end());
  }

  if ((resultString.length() > 0) && (resultString.at(0) == ' '))
  {
    resultString.erase(resultString.begin(), ++(resultString.begin()));
  }

  return resultString;
}

void mitk::CustomTagParser::SetParseStrategy(std::string parseStrategy)
{
  m_ParseStrategy = parseStrategy;
}

void mitk::CustomTagParser::SetRevisionMappingStrategy(std::string revisionMappingStrategy)
{
  m_RevisionMappingStrategy = revisionMappingStrategy;
}

std::string mitk::CustomTagParser::GetExternalJSONDirectory()
{
  std::string moduleLocation = us::GetModuleContext()->GetModule()->GetLocation();
  std::string stringToModule;
  std::string libraryName;
  itksys::SystemTools::SplitProgramPath(moduleLocation, stringToModule, libraryName);

  std::stringstream jsonDirectory;
  jsonDirectory << stringToModule << "/CESTRevisionMapping";

  return jsonDirectory.str();
}
