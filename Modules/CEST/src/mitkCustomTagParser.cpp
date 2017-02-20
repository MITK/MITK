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
}

mitk::PropertyList::Pointer mitk::CustomTagParser::ParseDicomPropertyString(std::string dicomPropertyString)
{
  auto results = mitk::PropertyList::New();
  if ("" == dicomPropertyString)
  {
    MITK_ERROR << "Could not parse empty custom dicom string";
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
    char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
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

  // determine what revision we are using to handle parameters appropriately
  std::string revisionPrefix = "CEST_Rev";

  std::size_t foundPosition = privateParameters["tSequenceFileName"].find(revisionPrefix);
  if (foundPosition == std::string::npos)
  {
    MITK_ERROR << "Could not find revision information.";
    return results;
  }

  std::string revisionString =
    privateParameters["tSequenceFileName"].substr(foundPosition + revisionPrefix.length(), std::string::npos);
  std::size_t firstNonNumber = revisionString.find_first_not_of("0123456789");
  revisionString.erase(firstNonNumber, std::string::npos);

  results->SetProperty(m_RevisionPropertyName, mitk::StringProperty::New(revisionString));

  std::string jsonString = GetRevisionAppropriateJSONString(revisionString);

  boost::property_tree::ptree root;
  std::istringstream jsonStream(jsonString);
  try
  {
    boost::property_tree::read_json(jsonStream, root);
  }
  catch (boost::property_tree::json_parser_error e)
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

  std::string sampling = "";
  std::string offset = "";
  std::string measurements = "";
  bool hasSamplingInformation = results->GetStringProperty("CEST.SamplingType", sampling);
  results->GetStringProperty("CEST.Offset", offset);
  results->GetStringProperty("CEST.measurements", measurements);

  if (hasSamplingInformation)
  {
    std::string offsets = GetOffsetString(sampling, offset, measurements);
    results->SetStringProperty(m_OffsetsPropertyName.c_str(), offsets.c_str());
  }
  else
  {
    MITK_WARN << "Could not determine sampling type.";
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
  std::string moduleLocation = us::GetModuleContext()->GetModule()->GetLocation();
  std::string stringToModule;
  std::string libraryName;
  itksys::SystemTools::SplitProgramPath(moduleLocation, stringToModule, libraryName);

  std::string prospectiveJsonsPath = stringToModule + "/*.json";

  std::set<std::string> JsonFiles;
  Poco::Glob::glob(prospectiveJsonsPath, JsonFiles);

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

std::string mitk::CustomTagParser::GetClosestLowerRevision(std::string revisionString, std::vector<int> &availableRevisionsVector)
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
      std::string moduleLocation = us::GetModuleContext()->GetModule()->GetLocation();
      std::string stringToModule;
      std::string libraryName;
      itksys::SystemTools::SplitProgramPath(moduleLocation, stringToModule, libraryName);

      std::string prospectiveJsonPath = stringToModule + "/" + m_ClosestExternalRevision + ".json";

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
        returnValue = std::string(std::istreambuf_iterator<char>(jsonStream), {});
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

      for (int index = 0; index < offsetVector.size(); ++index)
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
        currentOffset.erase(std::remove(currentOffset.begin(), currentOffset.end(), ' '), currentOffset.end());
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
  if ((resultString.length() > 0) && (resultString.at(resultString.length() - 1) == ' '))
  {
    resultString.erase(resultString.end() - 1, resultString.end());
  }

  return resultString;
}
