/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMonaiLabelTool.h"
#include <mitkIOUtil.h>
#include <httplib.h>
#include <filesystem>
#include <mitkPointSetShapeProperty.h>
#include <mitkProperties.h>
#include <mitkToolManager.h>
#include <mitkLabelSetImageHelper.h>

mitk::MonaiLabelTool::MonaiLabelTool() : SegWithPreviewTool(true, "PressMoveReleaseAndPointSetting") 
{
  this->ResetsToEmptyPreviewOn();
  this->IsTimePointChangeAwareOff();
}

mitk::MonaiLabelTool::~MonaiLabelTool()
{
  std::filesystem::remove_all(this->GetTempDir());
}

void mitk::MonaiLabelTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("ShiftSecondaryButtonPressed", OnAddNegativePoint);
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnAddPositivePoint);
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

void mitk::MonaiLabelTool::Activated()
{
  Superclass::Activated();
  m_PointSetPositive = mitk::PointSet::New();
  m_PointSetNodePositive = mitk::DataNode::New();
  m_PointSetNodePositive->SetData(m_PointSetPositive);
  m_PointSetNodePositive->SetName(std::string(this->GetName()) + "_PointSetPositive");
  m_PointSetNodePositive->SetBoolProperty("helper object", true);
  m_PointSetNodePositive->SetColor(0.0, 1.0, 0.0);
  m_PointSetNodePositive->SetVisibility(true);
  m_PointSetNodePositive->SetProperty("Pointset.2D.shape",
                                      mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::CIRCLE));
  m_PointSetNodePositive->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
  this->GetDataStorage()->Add(m_PointSetNodePositive, this->GetToolManager()->GetWorkingData(0));

  m_PointSetNegative = mitk::PointSet::New();
  m_PointSetNodeNegative = mitk::DataNode::New();
  m_PointSetNodeNegative->SetData(m_PointSetNegative);
  m_PointSetNodeNegative->SetName(std::string(this->GetName()) + "_PointSetNegative");
  m_PointSetNodeNegative->SetBoolProperty("helper object", true);
  m_PointSetNodeNegative->SetColor(1.0, 0.0, 0.0);
  m_PointSetNodeNegative->SetVisibility(true);
  m_PointSetNodeNegative->SetProperty("Pointset.2D.shape",
                                      mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::CIRCLE));
  m_PointSetNodeNegative->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
  this->GetDataStorage()->Add(m_PointSetNodeNegative, this->GetToolManager()->GetWorkingData(0));
}

void mitk::MonaiLabelTool::Deactivated()
{
  this->ClearSeeds();
  this->GetDataStorage()->Remove(m_PointSetNodePositive);
  this->GetDataStorage()->Remove(m_PointSetNodeNegative);
  m_PointSetNodePositive = nullptr;
  m_PointSetNodeNegative = nullptr;
  m_PointSetPositive = nullptr;
  m_PointSetNegative = nullptr;
  Superclass::Deactivated();
}

void mitk::MonaiLabelTool::UpdatePrepare()
{
  Superclass::UpdatePrepare();
  auto preview = this->GetPreviewSegmentation();
  preview->RemoveLabels(preview->GetAllLabelValues());
}

void mitk::MonaiLabelTool::OnDelete(StateMachineAction *, InteractionEvent *)
{
  if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
  {
    PointSet::Pointer removeSet = m_PointSetPositive;
    itk::IdentifierType maxId = 0;
    if (m_PointSetPositive->GetSize() > 0)
    {
      maxId = m_PointSetPositive->GetMaxId().Index();
    }
    if (m_PointSetNegative->GetSize() > 0 && (maxId < m_PointSetNegative->GetMaxId().Index()))
    {
      removeSet = m_PointSetNegative;
    }
    removeSet->RemovePointAtEnd(0);
    --m_PointSetCount;
    this->UpdatePreview();
  }
}

void mitk::MonaiLabelTool::ClearPicks()
{
  this->ClearSeeds();
  this->UpdatePreview();
}

bool mitk::MonaiLabelTool::HasPicks() const
{
  return this->m_PointSetPositive.IsNotNull() && this->m_PointSetPositive->GetSize() > 0;
}

void mitk::MonaiLabelTool::ClearSeeds()
{
  if (this->m_PointSetPositive.IsNotNull())
  {
    m_PointSetCount -= m_PointSetPositive->GetSize();
    this->m_PointSetPositive = mitk::PointSet::New(); // renew pointset
    this->m_PointSetNodePositive->SetData(this->m_PointSetPositive);
  }
  if (this->m_PointSetNegative.IsNotNull())
  {
    m_PointSetCount -= m_PointSetNegative->GetSize();
    this->m_PointSetNegative = mitk::PointSet::New(); // renew pointset
    this->m_PointSetNodeNegative->SetData(this->m_PointSetNegative);
  }
}

bool mitk::MonaiLabelTool::IsMonaiServerOn(const std::string &hostName, const int &port) const
{
  httplib::SSLClient cli(hostName, port);
  cli.enable_server_certificate_verification(false);
  while (cli.is_socket_open());
  return cli.Get("/info/");
}

void mitk::MonaiLabelTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                           const Image * /*oldSegAtTimeStep*/,
                                           LabelSetImage *previewImage,
                                           TimeStepType timeStep)
{
  const std::string &hostName = m_RequestParameters->hostName;
  const int port = m_RequestParameters->port;
  if (!IsMonaiServerOn(hostName, port))
  {
    mitkThrow() << m_SERVER_503_ERROR_TEXT;
  }
  if (this->m_TempDir.empty())
  {
    this->SetTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
  }

  std::string inputImagePath, outputImagePath;
  std::tie(inputImagePath, outputImagePath) = this->CreateTempDirs(m_TEMPLATE_FILENAME);

  try
  {
    this->WriteImage(inputAtTimeStep, inputImagePath);
    this->PostInferRequest(hostName, port, inputImagePath, outputImagePath, inputAtTimeStep->GetGeometry());
    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    auto outputBuffer = mitk::LabelSetImage::New();
    outputBuffer->InitializeByLabeledImage(outputImage);
    std::map<std::string, mitk::Label::PixelType> labelMap; // empty map
    if (m_RequestParameters->model.IsInteractive())
    {
      this->SetLabelTransferMode(LabelTransferMode::MapLabel);
      this->SetSelectedLabels({MASK_VALUE});
    }
    else
    {
      outputBuffer->SetGeometry(inputAtTimeStep->GetGeometry());
      labelMap = m_ResultMetadata["label_names"];
      this->SetLabelTransferMode(LabelTransferMode::AddLabel);
    }
    this->MapLabelsToSegmentation(outputBuffer, previewImage, labelMap);
    this->WriteBackResults(previewImage, outputBuffer.GetPointer(), timeStep);
    MonaiStatusEvent.Send(true);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    mitkThrow() << e.GetDescription();
    MonaiStatusEvent.Send(false);
  }
}

void mitk::MonaiLabelTool::MapLabelsToSegmentation(const mitk::LabelSetImage *source,
                                                   mitk::LabelSetImage *dest,
                                                   std::map<std::string, mitk::Label::PixelType> &labelMap)
{
  if (labelMap.empty())
  {
    auto label = LabelSetImageHelper::CreateNewLabel(dest, "object");
    label->SetValue(1);
    dest->AddLabel(label, false);
    return;
  }
  std::map<mitk::Label::PixelType, std::string> flippedLabelMap;
  for (auto const &[key, val] : labelMap)
  {
    flippedLabelMap[val] = key;
  }
  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(mitk::LookupTable::LookupTableType::MULTILABEL);
  for (auto const &[key, val] : flippedLabelMap)
  {
    if (source->ExistLabel(key, source->GetActiveLayer()))
    {
      Label::Pointer label = Label::New(key, val);
      std::array<double, 3> lookupTableColor;
      lookupTable->GetColor(key, lookupTableColor.data());
      Color color;
      color.SetRed(lookupTableColor[0]);
      color.SetGreen(lookupTableColor[1]);
      color.SetBlue(lookupTableColor[2]);
      label->SetColor(color);
      dest->AddLabel(label, false);
    }
    else
    {
      MITK_INFO << "Label not found for " << val;
    }
  }
}

void mitk::MonaiLabelTool::GetOverallInfo(const std::string &hostName, const int &port)
{
  if (!IsMonaiServerOn(hostName, port))
  {
    Tool::ErrorMessage.Send(m_SERVER_503_ERROR_TEXT);
    mitkThrow() << m_SERVER_503_ERROR_TEXT;
  }
  httplib::SSLClient cli(hostName, port);
  cli.enable_server_certificate_verification(false);
  if (auto response = cli.Get("/info/"))
  {
    if (response->status == 200)
    {
      auto jsonObj = nlohmann::json::parse(response->body);
      if (jsonObj.is_discarded() || !jsonObj.is_object())
      {
        MITK_ERROR << "Could not parse response from MONAILabel server as JSON object!";
        return;
      }
      m_InfoParameters = MapJSONToObject(jsonObj);
      if (nullptr != m_InfoParameters)
      {
        m_InfoParameters->hostName = hostName;
        m_InfoParameters->port = port;
      }
    }
  }
  else
  {
    Tool::ErrorMessage.Send(httplib::to_string(response.error()) + " error occured.");
  }
}

void mitk::MonaiLabelTool::PostInferRequest(const std::string &hostName,
                                            const int &port,
                                            std::string &filePath,
                                            std::string &outFile, 
                                            const mitk::BaseGeometry *baseGeometry)
{
  std::string &modelName = m_RequestParameters->model.name; // Get this from args as well.
  std::string postPath = "/infer/";                         // make this separate class of constants
  postPath.append(modelName);
  std::ifstream input(filePath, std::ios::binary);
  if (!input)
  {
    MITK_WARN << "could not read file to POST";
  }
  std::stringstream buffer_lf_img;
  buffer_lf_img << input.rdbuf();
  input.close();
  httplib::MultipartFormDataItems items;
  if (m_RequestParameters->model.IsInteractive())
  {
    std::stringstream foreground = this->GetPointsAsListString(baseGeometry, m_PointSetPositive);
    std::stringstream background = this->GetPointsAsListString(baseGeometry, m_PointSetNegative);
    std::stringstream paramString;
    paramString << "{" 
                << "\"foreground\":" << foreground.str() 
                << ",\"background\":" << background.str()
                << "}";
    MITK_DEBUG << paramString.str();
    items.push_back({"params", paramString.str(), "", ""});
  }
  else // Auto models
  {
    items.push_back({"params", "{\"restore_label_idx\": true}", "", ""});
  }
  items.push_back({"file", buffer_lf_img.str(), "post_from_mitk.nii.gz", "application/octet-stream"});
  httplib::SSLClient cli(hostName, port);
  cli.set_read_timeout(60);                      // arbitary 1 minute time-out to avoid corner cases.
  cli.enable_server_certificate_verification(false);
  if (auto response = cli.Post(postPath, items))
  {
    if (response->status == 200)
    {
      // Find boundary
      std::string boundaryString = this->GetBoundaryString(response);
      MITK_DEBUG << "boundary hash: " << boundaryString;

      // Parse metadata JSON
      std::string resBody = response->body;
      std::vector<std::string> multiPartResponse = this->GetPartsBetweenBoundary(resBody, boundaryString);

      std::string metaData = multiPartResponse[0];
      std::string contentTypeJson = "Content-Type: application/json";
      size_t ctPos = metaData.find(contentTypeJson) + contentTypeJson.length();
      std::string metaDataPart = metaData.substr(ctPos);
      MITK_DEBUG << metaDataPart;
      auto jsonObj = nlohmann::json::parse(metaDataPart);
      if (jsonObj.is_discarded() || !jsonObj.is_object())
      {
        MITK_ERROR << "Could not parse response from MONAILabel server as JSON object!";
        return;
      }
      else // use the metadata
      {
        m_ResultMetadata = jsonObj;
      }
      // Parse response image string
      std::string imagePart = multiPartResponse[1];
      std::string imageData = GetResponseImageString(imagePart);
      std::ofstream output(outFile, std::ios::out | std::ios::app | std::ios::binary);
      output << imageData;
      output.unsetf(std::ios::skipws);
      output.flush();
    }
    else
    {
      auto err = response.error();
      MITK_ERROR << "An HTTP POST error: " << httplib::to_string(err) << " occured.";
      mitkThrow() << "An HTTP POST error: " << httplib::to_string(err) << " occured.";
    }
  }
}

std::string mitk::MonaiLabelTool::GetBoundaryString(httplib::Result& response)
{
  httplib::Headers headers = response->headers;
  std::string contentType = headers.find("content-type")->second;
  std::string delimiter = "boundary=";
  std::string boundaryString = contentType.substr(contentType.find(delimiter) + delimiter.length(), std::string::npos);
  boundaryString.insert(0, "--");
  return boundaryString;
}

std::string mitk::MonaiLabelTool::GetResponseImageString(std::string &imagePart)
{
  std::string contentTypeStream = "Content-Type: application/octet-stream";
  size_t ctPos = imagePart.find(contentTypeStream) + contentTypeStream.length();
  std::string imageDataPart = imagePart.substr(ctPos);
  std::string whitespaces = " \n\r";
  imageDataPart.erase(0, imageDataPart.find_first_not_of(whitespaces)); // clean up
  return imageDataPart;
}

std::vector<std::string> mitk::MonaiLabelTool::GetPartsBetweenBoundary(const std::string &body,
                                                                       const std::string &boundary)
{
  std::vector<std::string> retVal;
  std::string master = body;
  size_t boundaryPos = master.find(boundary);
  size_t beginPos = 0;
  while (boundaryPos != std::string::npos)
  {
    std::string part = master.substr(beginPos, boundaryPos);
    if (!part.empty())
    {
      retVal.push_back(part);
    }
    master.erase(beginPos, boundaryPos + boundary.length());
    boundaryPos = master.find(boundary);
  }
  return retVal;
}

std::unique_ptr<mitk::MonaiAppMetadata> mitk::MonaiLabelTool::MapJSONToObject(nlohmann::json &jsonObj)
{
  mitk::MonaiAppMetadata appData{
    jsonObj["name"].template get<std::string>(),
    jsonObj["description"].template get<std::string>(),
    jsonObj["labels"].template get<std::vector<std::string>>()
  };
  auto object = std::make_unique<MonaiAppMetadata>(appData);
  auto modelJsonMap = jsonObj["models"].get<std::map<std::string, nlohmann::json>>();

  for (const auto &[_name, _jsonObj] : modelJsonMap)
  {
    if (_jsonObj.is_discarded() || !_jsonObj.is_object())
    {
      MITK_ERROR << "Could not parse JSON object.";
    }
    MonaiModelInfo modelInfo;
    modelInfo.name = _name;
    try
    {
      auto labels = _jsonObj["labels"].get<std::unordered_map<std::string, int>>();
      modelInfo.labels = labels;
    }
    catch (const std::exception &)
    {
      auto labels = _jsonObj["labels"].get<std::vector<std::string>>();
      for (const auto &label : labels)
      {
        modelInfo.labels[label] = -1; // Hardcode -1 as label id
      }
    }
    modelInfo.type = _jsonObj["type"].get<std::string>();
    modelInfo.dimension = _jsonObj["dimension"].get<int>();
    modelInfo.description = _jsonObj["description"].get<std::string>();

    object->models.push_back(modelInfo);
  }
  return object;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetAutoSegmentationModels(int dim) const
{
  std::vector<mitk::MonaiModelInfo> autoModels;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_AUTO_SEG_TYPE_NAME.find(model.type) != m_AUTO_SEG_TYPE_NAME.end() && (!(dim > -1) ||
          model.dimension == dim))
      {
        autoModels.push_back(model);
      }
    }
  }
  return autoModels;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetInteractiveSegmentationModels(int dim) const
{
  std::vector<mitk::MonaiModelInfo> interactiveModels;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_INTERACTIVE_SEG_TYPE_NAME.find(model.type) != m_INTERACTIVE_SEG_TYPE_NAME.end() &&
          (!(dim > -1) || model.dimension == dim))
      {
        interactiveModels.push_back(model);
      }
    }
  }
  return interactiveModels;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetScribbleSegmentationModels(int dim) const
{
  std::vector<mitk::MonaiModelInfo> scribbleModels;
  bool checkDims = dim > -1;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_SCRIBBLE_SEG_TYPE_NAME.find(model.type) != m_SCRIBBLE_SEG_TYPE_NAME.end() &&
          (!(dim > -1) || model.dimension == dim))
      {
        scribbleModels.push_back(model);
      }
    }
  }
  return scribbleModels;
}

mitk::MonaiModelInfo mitk::MonaiLabelTool::GetModelInfoFromName(const std::string modelName) const
{
  if (nullptr == m_InfoParameters)
  {
    mitkThrow() << "No model information found.";
  }
  mitk::MonaiModelInfo retVal;
  for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
  {
    if (model.name == modelName)
    {
      retVal = model;
      break;
    }
  }
  return retVal;
}

void mitk::MonaiLabelTool::WriteImage(const Image*, std::string&) {}

std::pair<std::string, std::string> mitk::MonaiLabelTool::CreateTempDirs(const std::string &filePattern)
{
  std::string inDir, outDir, inputImagePath, outputImagePath;
  inDir = IOUtil::CreateTemporaryDirectory("monai-in-XXXXXX", this->GetTempDir());
  std::ofstream tmpStream;
  inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, filePattern, inDir + IOUtil::GetDirectorySeparator());
  tmpStream.close();
  std::size_t found = inputImagePath.find_last_of(IOUtil::GetDirectorySeparator());
  std::string fileName = inputImagePath.substr(found + 1);
  std::string token = fileName.substr(0, fileName.find("_"));
  outDir = IOUtil::CreateTemporaryDirectory("monai-out-XXXXXX", this->GetTempDir());
  outputImagePath = outDir + IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
  return std::make_pair(inputImagePath, outputImagePath);
}
