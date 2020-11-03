/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCLResultXMLWriter.h>

#include <iostream>
#include <locale>
#include <iomanip>
#include <ctime>
#include <sstream>

#include <mitkDICOMTagPath.h>
#include <mitkDICOMProperty.h>
#include <mitkLocaleSwitch.h>
#include <mitkIOMetaInformationPropertyConstants.h>
#include <mitkDICOMIOMetaInformationPropertyConstants.h>

#include <tinyxml.h>

template <class charT>
class punct_facet : public std::numpunct<charT> {
public:
  punct_facet(charT sep) :
    m_Sep(sep)
  {

  }
protected:
  charT do_decimal_point() const override { return m_Sep; }
private:
  charT m_Sep;
};

void AddPropertyAsNode(const mitk::Image* image, const std::string& key, const std::string& tag, TiXmlElement* rootNode)
{
  auto prop = image->GetProperty(key.c_str());
  if (prop.IsNotNull())
  {
    auto propNode = new TiXmlElement(tag);
    TiXmlText* valueText = new TiXmlText(prop->GetValueAsString());
    propNode->LinkEndChild(valueText);

    rootNode->LinkEndChild(propNode);
  }
}

void AddSeriesInstanceUID(const mitk::Image* image, TiXmlElement* xmlNode)
{
  AddPropertyAsNode(image, mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0020, 0x000e)), "mp:seriesInstanceUID", xmlNode);
}

void AddFilePath(const mitk::Image* image, TiXmlElement* xmlNode)
{
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_INPUTLOCATION()), "mp:filePath", xmlNode);
}

void AddSOPInstanceUIDs(const mitk::Image* image, TiXmlElement* xmlNode)
{
  auto prop = image->GetProperty(mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0008, 0x0018)).c_str());
  auto dicomProp = dynamic_cast<const mitk::DICOMProperty*>(prop.GetPointer());
  if (dicomProp != nullptr)
  {
    auto instanceUIDsNode = new TiXmlElement("mp:sopInstanceUIDs");
    xmlNode->LinkEndChild(instanceUIDsNode);

    if (dicomProp->IsUniform())
    {
      auto instanceUIDNode = new TiXmlElement("mp:sopInstanceUID");
      TiXmlText* valueText = new TiXmlText(dicomProp->GetValueAsString());
      instanceUIDNode->LinkEndChild(valueText);
      instanceUIDsNode->LinkEndChild(instanceUIDNode);
    }
    else
    {
      const auto timeSteps = dicomProp->GetAvailableTimeSteps();
      for (auto timeStep : timeSteps)
      {
        const auto slices = dicomProp->GetAvailableSlices(timeStep);
        for (auto slice : slices)
        {
          auto instanceUIDNode = new TiXmlElement("mp:sopInstanceUID");
          instanceUIDNode->SetAttribute("z", slice);
          instanceUIDNode->SetAttribute("t", timeStep);
          TiXmlText* valueText = new TiXmlText(dicomProp->GetValue(timeStep, slice));
          instanceUIDNode->LinkEndChild(valueText);
          instanceUIDsNode->LinkEndChild(instanceUIDNode);
        }
      }
    }
  }
}

void AddDateAndTime(TiXmlElement* rootNode)
{
  auto dateNode = new TiXmlElement("mp:generationDate");
  auto time = std::time(nullptr);
  std::ostringstream sstream;
  sstream << std::put_time(std::localtime(&time), "%Y%m%d");
  TiXmlText* valueText = new TiXmlText(sstream.str());
  dateNode->LinkEndChild(valueText);

  rootNode->LinkEndChild(dateNode);

  auto timeNode = new TiXmlElement("mp:generationTime");
  std::ostringstream timestream;
  timestream << std::put_time(std::localtime(&time), "%H%M%S");
  valueText = new TiXmlText(timestream.str());
  timeNode->LinkEndChild(valueText);

  rootNode->LinkEndChild(timeNode);
}

void AddParameters(const std::map<std::string, us::Any>& parameters, TiXmlElement* paramsNode)
{
  for (const auto& param : parameters)
  {
    mitk::LocaleSwitch lswitch("C");
    auto paramNode = new TiXmlElement("mp:parameter");
    paramNode->SetAttribute("name", param.first);
    TiXmlText* valueText = new TiXmlText(param.second.ToString());
    paramNode->LinkEndChild(valueText);
    paramsNode->LinkEndChild(paramNode);
  }
}

void AddFeatures(const mitk::AbstractGlobalImageFeature::FeatureListType& features, TiXmlElement* featsNode)
{
  for (const auto& feat : features)
  {
    auto featNode = new TiXmlElement("mp:feature");
    featNode->SetAttribute("name", feat.first.name);
    featNode->SetAttribute("version", feat.first.version);
    featNode->SetAttribute("class", feat.first.featureClass);
    featNode->SetAttribute("setting", feat.first.settingID);
    std::ostringstream sstream;
    sstream.imbue(std::locale("C"));
    sstream << feat.second;
    TiXmlText* valueText = new TiXmlText(sstream.str());
    featNode->LinkEndChild(valueText);
    featsNode->LinkEndChild(featNode);
  }
}

void AddFeatureSettings(const mitk::AbstractGlobalImageFeature::FeatureListType& features, TiXmlElement* featSettingsNode)
{
  std::list<std::string> coveredSettings;
  for (const auto& feat : features)
  {
    auto finding = std::find(coveredSettings.begin(), coveredSettings.end(), feat.first.settingID);
    if (finding == coveredSettings.end())
    {
      auto featSettingNode = new TiXmlElement("mp:featureSetting");
      featSettingsNode->LinkEndChild(featSettingNode);
      featSettingNode->SetAttribute("name", feat.first.settingID);
      AddParameters(feat.first.parameters, featSettingNode);
      coveredSettings.push_back(feat.first.settingID);
    }
  }
}

void AddReaderInfo(const mitk::Image* image, TiXmlElement* imageNode)
{
  auto ioNode = new TiXmlElement("mp:IOReader");
  imageNode->LinkEndChild(ioNode);
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_DESCRIPTION()), "mp:description", ioNode);
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_VERSION()), "mp:version", ioNode);
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_CONFIGURATION()), "mp:configuration", ioNode);
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_GDCM()), "mp:gdcmVersion", ioNode);
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_DCMTK()), "mp:dcmtkVersion", ioNode);
}

void WriteDocument(std::ostream& stream, const mitk::Image* image, const mitk::Image* mask,
  const mitk::AbstractGlobalImageFeature::FeatureListType& features, const std::string& methodName,
  const std::string& organisation, const std::string& version, const std::string& pipelineUID,
  const std::map<std::string, us::Any>& cliArgs)
{
  TiXmlDocument doc;
  doc.SetCondenseWhiteSpace(false);

  auto decl = new TiXmlDeclaration(
    "1.0", "UTF-8", "");
  doc.LinkEndChild(decl);

  auto rootNode = new TiXmlElement("mp:measurement");
  rootNode->SetAttribute("xmlns:mp", "http://www.mitk.org/Phenotyping");
  doc.LinkEndChild(rootNode);

  auto methodNode = new TiXmlElement("mp:measurementMethod");
  rootNode->LinkEndChild(methodNode);

  auto methodNameNode = new TiXmlElement("mp:name");
  TiXmlText* valueText = new TiXmlText(methodName);
  methodNameNode->LinkEndChild(valueText);
  methodNode->LinkEndChild(methodNameNode);

  auto organisationNode = new TiXmlElement("mp:organisation");
  valueText = new TiXmlText(organisation);
  organisationNode->LinkEndChild(valueText);
  methodNode->LinkEndChild(organisationNode);

  auto versionNode = new TiXmlElement("mp:version");
  valueText = new TiXmlText(version);
  versionNode->LinkEndChild(valueText);
  methodNode->LinkEndChild(versionNode);

  auto imageNode = new TiXmlElement("mp:image");
  rootNode->LinkEndChild(imageNode);

  AddSeriesInstanceUID(image, imageNode);
  AddFilePath(image, imageNode);
  AddSOPInstanceUIDs(image, imageNode);
  AddReaderInfo(image,imageNode);


  auto maskNode = new TiXmlElement("mp:mask");
  rootNode->LinkEndChild(maskNode);

  AddSeriesInstanceUID(mask, maskNode);
  AddFilePath(mask, maskNode);
  AddSOPInstanceUIDs(mask, maskNode);
  AddReaderInfo(mask, maskNode);

  //todo mask reader meta info

  AddDateAndTime(rootNode);

  auto pipelineNode = new TiXmlElement("mp:pipelineUID");
  valueText = new TiXmlText(pipelineUID);
  pipelineNode->LinkEndChild(valueText);
  rootNode->LinkEndChild(pipelineNode);

  auto paramsNode = new TiXmlElement("mp:parameters");
  rootNode->LinkEndChild(paramsNode);
  AddParameters(cliArgs, paramsNode);

  auto featsNode = new TiXmlElement("mp:features");
  rootNode->LinkEndChild(featsNode);

  AddFeatures(features, featsNode);

  auto featSettingsNode = new TiXmlElement("mp:featureSettings");
  rootNode->LinkEndChild(featSettingsNode);
  AddFeatureSettings(features, featSettingsNode);

  TiXmlPrinter printer;

  doc.Accept(&printer);
  stream << printer.Str();
}

void mitk::cl::CLResultXMLWriter::SetImage(const Image* image)
{
  m_Image = image;
}

void mitk::cl::CLResultXMLWriter::SetMask(const Image* mask)
{
  m_Mask = mask;
}

void mitk::cl::CLResultXMLWriter::SetFeatures(const mitk::AbstractGlobalImageFeature::FeatureListType& features)
{
  m_Features = features;
}

void mitk::cl::CLResultXMLWriter::SetMethodName(const std::string& name)
{
  m_MethodName = name;
}

void mitk::cl::CLResultXMLWriter::SetMethodVersion(const std::string& version)
{
  m_MethodVersion = version;
}

void mitk::cl::CLResultXMLWriter::SetOrganisation(const std::string& orga)
{
  m_Organisation = orga;
}

void mitk::cl::CLResultXMLWriter::SetPipelineUID(const std::string& pipelineUID)
{
  m_PipelineUID = pipelineUID;
}

void mitk::cl::CLResultXMLWriter::SetCLIArgs(const std::map<std::string, us::Any>& args)
{
  m_CLIArgs = args;
}

void mitk::cl::CLResultXMLWriter::write(const std::string& filePath) const
{
  std::ofstream resultFile;
  resultFile.open(filePath.c_str());
  if (resultFile.is_open())
  {
    this->write(resultFile);
    resultFile.close();
  }
  else
  {
    MITK_ERROR << "Cannot write xml results. Unable to open file: \""<<filePath<<"\"";
  }
}

void mitk::cl::CLResultXMLWriter::write(std::ostream& stream) const
{
  WriteDocument(stream, m_Image, m_Mask, m_Features, m_MethodName, m_Organisation, m_MethodVersion, m_PipelineUID, m_CLIArgs);
}
