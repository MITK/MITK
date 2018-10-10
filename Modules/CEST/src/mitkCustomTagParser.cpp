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

#include "mitkCustomTagParser.h"

#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkLocaleSwitch.h>

#include "mitkIPropertyPersistence.h"
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include <itksys/SystemTools.hxx>

#include <Poco/Glob.h>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <map>

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
const std::string mitk::CustomTagParser::m_OffsetsPropertyName = m_CESTPropertyPrefix + "Offsets";
const std::string mitk::CustomTagParser::m_RevisionPropertyName = m_CESTPropertyPrefix + "Revision";
const std::string mitk::CustomTagParser::m_JSONRevisionPropertyName = m_CESTPropertyPrefix + "revision_json";

const std::string mitk::CustomTagParser::m_RevisionIndependentMapping =
"\n"
"  \"sProtConsistencyInfo.tSystemType\" : \"SysType\",\n"
"  \"sProtConsistencyInfo.flNominalB0\" : \"NominalB0\",\n"
"  \"sTXSPEC.asNucleusInfo[0].lFrequency\" : \"FREQ\",\n"
"  \"sTXSPEC.asNucleusInfo[0].flReferenceAmplitude\" : \"RefAmp\",\n"
"  \"alTR[0]\" : \"TR\",\n"
"  \"alTE[0]\" : \"TE\",\n"
"  \"lAverages\" : \"averages\",\n"
"  \"lRepetitions\" : \"repetitions\",\n"
"  \"adFlipAngleDegree[0]\" : \"ImageFlipAngle\",\n"
"  \"lTotalScanTimeSec\" : \"TotalScanTime\",";
const std::string mitk::CustomTagParser::m_DefaultJsonString =
"{\n"
"  \"default mapping, corresponds to revision 1416\" : \"revision_json\",\n"
"  \"sWiPMemBlock.alFree[1]\" : \"AdvancedMode\",\n"
"  \"sWiPMemBlock.alFree[2]\" : \"RecoveryMode\",\n"
"  \"sWiPMemBlock.alFree[3]\" : \"DoubleIrrMode\",\n"
"  \"sWiPMemBlock.alFree[4]\" : \"BinomMode\",\n"
"  \"sWiPMemBlock.alFree[5]\" : \"MtMode\",\n"
"  \"sWiPMemBlock.alFree[6]\" : \"PreparationType\",\n"
"  \"sWiPMemBlock.alFree[7]\" : \"PulseType\",\n"
"  \"sWiPMemBlock.alFree[8]\" : \"SamplingType\",\n"
"  \"sWiPMemBlock.alFree[9]\" : \"SpoilingType\",\n"
"  \"sWiPMemBlock.alFree[10]\" : \"measurements\",\n"
"  \"sWiPMemBlock.alFree[11]\" : \"NumberOfPulses\",\n"
"  \"sWiPMemBlock.alFree[12]\" : \"NumberOfLockingPulses\",\n"
"  \"sWiPMemBlock.alFree[13]\" : \"PulseDuration\",\n"
"  \"sWiPMemBlock.alFree[14]\" : \"DutyCycle\",\n"
"  \"sWiPMemBlock.alFree[15]\" : \"RecoveryTime\",\n"
"  \"sWiPMemBlock.alFree[16]\" : \"RecoveryTimeM0\",\n"
"  \"sWiPMemBlock.alFree[17]\" : \"ReadoutDelay\",\n"
"  \"sWiPMemBlock.alFree[18]\" : \"BinomDuration\",\n"
"  \"sWiPMemBlock.alFree[19]\" : \"BinomDistance\",\n"
"  \"sWiPMemBlock.alFree[20]\" : \"BinomNumberofPulses\",\n"
"  \"sWiPMemBlock.alFree[21]\" : \"BinomPreRepetions\",\n"
"  \"sWiPMemBlock.alFree[22]\" : \"BinomType\",\n"
"  \"sWiPMemBlock.adFree[1]\" : \"Offset\",\n"
"  \"sWiPMemBlock.adFree[2]\" : \"B1Amplitude\",\n"
"  \"sWiPMemBlock.adFree[3]\" : \"AdiabaticPulseMu\",\n"
"  \"sWiPMemBlock.adFree[4]\" : \"AdiabaticPulseBW\",\n"
"  \"sWiPMemBlock.adFree[5]\" : \"AdiabaticPulseLength\",\n"
"  \"sWiPMemBlock.adFree[6]\" : \"AdiabaticPulseAmp\",\n"
"  \"sWiPMemBlock.adFree[7]\" : \"FermiSlope\",\n"
"  \"sWiPMemBlock.adFree[8]\" : \"FermiFWHM\",\n"
"  \"sWiPMemBlock.adFree[9]\" : \"DoubleIrrDuration\",\n"
"  \"sWiPMemBlock.adFree[10]\" : \"DoubleIrrAmplitude\",\n"
"  \"sWiPMemBlock.adFree[11]\" : \"DoubleIrrRepetitions\",\n"
"  \"sWiPMemBlock.adFree[12]\" : \"DoubleIrrPreRepetitions\"\n"
"}";

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

  std::map<std::string, std::string> privateParameters;

  // convert hex to ascii
  // the Siemens private tag contains the information like this
  // "43\52\23\34" we jump over each \ and convert the number
  int len = dicomPropertyString.length();
  std::string asciiString;
  for (int i = 0; i < len; i += 3)
  {
    std::string byte = dicomPropertyString.substr(i, 2);
    auto chr = (char)(int)strtol(byte.c_str(), nullptr, 16);
    asciiString.push_back(chr);
  }

  // extract parameter list
  std::size_t beginning = asciiString.find("### ASCCONV BEGIN ###") + 21;
  std::size_t ending = asciiString.find("### ASCCONV END ###");
  std::string parameterListString = asciiString.substr(beginning, ending - beginning);

  boost::replace_all(parameterListString, "\r\n", "\n");
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

  boost::property_tree::ptree root;
  std::istringstream jsonStream(jsonString);
  try
  {
    boost::property_tree::read_json(jsonStream, root);
  }
  catch (const boost::property_tree::json_parser_error &e)
  {
    mitkThrow() << "Could not parse json file. Error was:\n" << e.what();
  }

  for (auto it : root)
  {
    if (it.second.empty())
    {
      std::string propertyName = m_CESTPropertyPrefix + it.second.data();
      if (m_JSONRevisionPropertyName == propertyName)
      {
        results->SetProperty(propertyName, mitk::StringProperty::New(it.first));
      }
      else
      {
        results->SetProperty(propertyName, mitk::StringProperty::New(privateParameters[it.first]));
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

  if ("" == measurements)
  {
    std::string stringRepetitions = "";
    std::string stringAverages = "";
    results->GetStringProperty("CEST.repetitions", stringRepetitions);
    results->GetStringProperty("CEST.averages", stringAverages);
    std::stringstream  measurementStream;
    try
    {
      measurementStream << std::stoi(stringRepetitions) + std::stoi(stringAverages);
      measurements = measurementStream.str();
      MITK_INFO << "Could not find measurements, assuming repetitions + averages. Which is: " << measurements;
    }
    catch (const std::invalid_argument &ia)
    {
      MITK_ERROR
        << "Could not find measurements, fallback assumption of repetitions + averages could not be determined either: "
        << ia.what();
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

    mitk::LocaleSwitch localeSwitch("C");

    std::stringstream trecStream;

    std::string trecPath = m_DicomDataPath + "/TREC.txt";
    std::ifstream list(trecPath.c_str());

    if (list.good())
    {
      std::string currentTime;
      while (std::getline(list, currentTime))
      {
        trecStream << currentTime << " ";
      }
    }
    else
    {
      MITK_WARN << "Assumed T1, but could not load TREC at " << trecPath;
    }

    results->SetStringProperty(CEST_PROPERTY_NAME_TREC().c_str(), trecStream.str().c_str());
  }
  else
  {
    MITK_INFO << "Parsed as CEST or WASABI image";
    std::string sampling = "";
    bool hasSamplingInformation = results->GetStringProperty("CEST.SamplingType", sampling);
    if (hasSamplingInformation)
    {
      std::string offsets = GetOffsetString(sampling, offset, measurements);
      results->SetStringProperty(m_OffsetsPropertyName.c_str(), offsets.c_str());
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

  for (auto const resource : configs)
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

  for (auto const jsonpath : JsonFiles)
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
  mitk::LocaleSwitch localeSwitch("C");
  std::stringstream results;

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
    std::ifstream list(listPath.c_str());

    if (list.good())
    {
      std::string currentOffset;
      while (std::getline(list, currentOffset))
      {
        results << currentOffset << " ";
      }
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

const std::string mitk::CEST_PROPERTY_NAME_TOTALSCANTIME()
{
  return "CEST.TotalScanTime";
};

const std::string mitk::CEST_PROPERTY_NAME_PREPERATIONTYPE()
{
  return "CEST.PreparationType";
};

const std::string mitk::CEST_PROPERTY_NAME_RECOVERYMODE()
{
  return "CEST.RecoveryMode";
};

const std::string mitk::CEST_PROPERTY_NAME_SPOILINGTYPE()
{
  return "CEST.SpoilingType";
};

const std::string mitk::CEST_PROPERTY_NAME_OFFSETS()
{
  return "CEST.Offsets";
};

const std::string mitk::CEST_PROPERTY_NAME_TREC()
{
  return std::string("CEST.TREC");
}
