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

#include <tinyxml2.h>

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

void AddPropertyAsNode(const mitk::Image* image, const std::string& key, const std::string& tag, tinyxml2::XMLElement* rootNode)
{
  auto prop = image->GetProperty(key.c_str());
  if (prop.IsNotNull())
  {
    auto* doc = rootNode->GetDocument();
    auto propNode = doc->NewElement(tag.c_str());
    auto* valueText = doc->NewText(prop->GetValueAsString().c_str());
    propNode->InsertEndChild(valueText);

    rootNode->InsertEndChild(propNode);
  }
}

void AddSeriesInstanceUID(const mitk::Image* image, tinyxml2::XMLElement* xmlNode)
{
  AddPropertyAsNode(image, mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0020, 0x000e)), "mp:seriesInstanceUID", xmlNode);
}

void AddFilePath(const mitk::Image* image, tinyxml2::XMLElement* xmlNode)
{
  AddPropertyAsNode(image, mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_INPUTLOCATION()), "mp:filePath", xmlNode);
}

void AddSOPInstanceUIDs(const mitk::Image* image, tinyxml2::XMLElement* xmlNode)
{
  auto prop = image->GetProperty(mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0008, 0x0018)).c_str());
  auto dicomProp = dynamic_cast<const mitk::DICOMProperty*>(prop.GetPointer());
  auto* doc = xmlNode->GetDocument();
  if (dicomProp != nullptr)
  {
    auto* instanceUIDsNode = doc->NewElement("mp:sopInstanceUIDs");
    xmlNode->InsertEndChild(instanceUIDsNode);

    if (dicomProp->IsUniform())
    {
      auto* instanceUIDNode = doc->NewElement("mp:sopInstanceUID");
      auto* valueText = doc->NewText(dicomProp->GetValueAsString().c_str());
      instanceUIDNode->InsertEndChild(valueText);
      instanceUIDsNode->InsertEndChild(instanceUIDNode);
    }
    else
    {
      const auto timeSteps = dicomProp->GetAvailableTimeSteps();
      for (auto timeStep : timeSteps)
      {
        const auto slices = dicomProp->GetAvailableSlices(timeStep);
        for (auto slice : slices)
        {
          auto instanceUIDNode = doc->NewElement("mp:sopInstanceUID");
          instanceUIDNode->SetAttribute("z", static_cast<int>(slice));
          instanceUIDNode->SetAttribute("t", static_cast<int>(timeStep));
          auto* valueText = doc->NewText(dicomProp->GetValue(timeStep, slice).c_str());
          instanceUIDNode->InsertEndChild(valueText);
          instanceUIDsNode->InsertEndChild(instanceUIDNode);
        }
      }
    }
  }
}

void AddDateAndTime(tinyxml2::XMLElement* rootNode)
{
  auto* doc = rootNode->GetDocument();
  auto* dateNode = doc->NewElement("mp:generationDate");
  auto time = std::time(nullptr);
  std::ostringstream sstream;
  sstream << std::put_time(std::localtime(&time), "%Y%m%d");
  auto* valueText = doc->NewText(sstream.str().c_str());
  dateNode->InsertEndChild(valueText);

  rootNode->InsertEndChild(dateNode);

  auto* timeNode = doc->NewElement("mp:generationTime");
  std::ostringstream timestream;
  timestream << std::put_time(std::localtime(&time), "%H%M%S");
  valueText = doc->NewText(timestream.str().c_str());
  timeNode->InsertEndChild(valueText);

  rootNode->InsertEndChild(timeNode);
}

void AddParameters(const std::map<std::string, us::Any>& parameters, tinyxml2::XMLElement* paramsNode)
{
  auto* doc = paramsNode->GetDocument();
  for (const auto& param : parameters)
  {
    mitk::LocaleSwitch lswitch("C");
    auto paramNode = doc->NewElement("mp:parameter");
    paramNode->SetAttribute("name", param.first.c_str());
    auto* valueText = doc->NewText(param.second.ToString().c_str());
    paramNode->InsertEndChild(valueText);
    paramsNode->InsertEndChild(paramNode);
  }
}

void AddFeatures(const mitk::AbstractGlobalImageFeature::FeatureListType& features, tinyxml2::XMLElement* featsNode)
{
  auto* doc = featsNode->GetDocument();
  for (const auto& feat : features)
  {
    auto featNode = doc->NewElement("mp:feature");
    featNode->SetAttribute("name", feat.first.name.c_str());
    featNode->SetAttribute("version", feat.first.version.c_str());
    featNode->SetAttribute("class", feat.first.featureClass.c_str());
    featNode->SetAttribute("setting", feat.first.settingID.c_str());
    std::ostringstream sstream;
    sstream.imbue(std::locale("C"));
    sstream << feat.second;
    auto* valueText = doc->NewText(sstream.str().c_str());
    featNode->InsertEndChild(valueText);
    featsNode->InsertEndChild(featNode);
  }
}

void AddFeatureSettings(const mitk::AbstractGlobalImageFeature::FeatureListType& features, tinyxml2::XMLElement* featSettingsNode)
{
  auto* doc = featSettingsNode->GetDocument();
  std::list<std::string> coveredSettings;
  for (const auto& feat : features)
  {
    auto finding = std::find(coveredSettings.begin(), coveredSettings.end(), feat.first.settingID);
    if (finding == coveredSettings.end())
    {
      auto featSettingNode = doc->NewElement("mp:featureSetting");
      featSettingsNode->InsertEndChild(featSettingNode);
      featSettingNode->SetAttribute("name", feat.first.settingID.c_str());
      AddParameters(feat.first.parameters, featSettingNode);
      coveredSettings.push_back(feat.first.settingID);
    }
  }
}

void AddReaderInfo(const mitk::Image* image, tinyxml2::XMLElement* imageNode)
{
  auto* doc = imageNode->GetDocument();
  auto ioNode = doc->NewElement("mp:IOReader");
  imageNode->InsertEndChild(ioNode);
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
  tinyxml2::XMLDocument doc;
  doc.InsertEndChild(doc.NewDeclaration());

  auto* rootNode = doc.NewElement("mp:measurement");
  rootNode->SetAttribute("xmlns:mp", "http://www.mitk.org/Phenotyping");
  doc.InsertEndChild(rootNode);

  auto* methodNode = doc.NewElement("mp:measurementMethod");
  rootNode->InsertEndChild(methodNode);

  auto* methodNameNode = doc.NewElement("mp:name");
  auto* valueText = doc.NewText(methodName.c_str());
  methodNameNode->InsertEndChild(valueText);
  methodNode->InsertEndChild(methodNameNode);

  auto* organisationNode = doc.NewElement("mp:organisation");
  valueText = doc.NewText(organisation.c_str());
  organisationNode->InsertEndChild(valueText);
  methodNode->InsertEndChild(organisationNode);

  auto* versionNode = doc.NewElement("mp:version");
  valueText = doc.NewText(version.c_str());
  versionNode->InsertEndChild(valueText);
  methodNode->InsertEndChild(versionNode);

  auto* imageNode = doc.NewElement("mp:image");
  rootNode->InsertEndChild(imageNode);

  AddSeriesInstanceUID(image, imageNode);
  AddFilePath(image, imageNode);
  AddSOPInstanceUIDs(image, imageNode);
  AddReaderInfo(image,imageNode);


  auto* maskNode = doc.NewElement("mp:mask");
  rootNode->InsertEndChild(maskNode);

  AddSeriesInstanceUID(mask, maskNode);
  AddFilePath(mask, maskNode);
  AddSOPInstanceUIDs(mask, maskNode);
  AddReaderInfo(mask, maskNode);

  //todo mask reader meta info

  AddDateAndTime(rootNode);

  auto* pipelineNode = doc.NewElement("mp:pipelineUID");
  valueText = doc.NewText(pipelineUID.c_str());
  pipelineNode->InsertEndChild(valueText);
  rootNode->InsertEndChild(pipelineNode);

  auto* paramsNode = doc.NewElement("mp:parameters");
  rootNode->InsertEndChild(paramsNode);
  AddParameters(cliArgs, paramsNode);

  auto* featsNode = doc.NewElement("mp:features");
  rootNode->InsertEndChild(featsNode);

  AddFeatures(features, featsNode);

  auto* featSettingsNode = doc.NewElement("mp:featureSettings");
  rootNode->InsertEndChild(featSettingsNode);
  AddFeatureSettings(features, featSettingsNode);

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);

  stream << printer.CStr();
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
