/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMonaiLabelTool.h"
#include "mitkIOUtil.h"
#include <httplib.h>
#include <itksys/SystemTools.hxx>
#include <mitkImageReadAccessor.h>
#include <nlohmann/json.hpp>

mitk::MonaiLabelTool::MonaiLabelTool()
{
  this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
}

mitk::MonaiLabelTool::~MonaiLabelTool()
{
  itksys::SystemTools::RemoveADirectory(this->GetMitkTempDir());
}

bool mitk::MonaiLabelTool::IsMonaiServerOn(std::string &hostName, int &port)
{
  httplib::Client cli(hostName, port);
  while (cli.is_socket_open())
    ;
  if (auto response = cli.Get("/info/"))
  {
    return true;
  }
  return false;
}

void mitk::MonaiLabelTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                           const Image * /*oldSegAtTimeStep*/,
                                           LabelSetImage *previewImage,
                                           TimeStepType timeStep)
{
  std::string &hostName = m_RequestParameters->hostName;
  int port = m_RequestParameters->port;
  if (!m_TEST)
    if (!IsMonaiServerOn(hostName, port))
    {
      mitkThrow() << m_SERVER_503_ERROR_TEXT;
    }
  std::string inDir, outDir, inputImagePath, outputImagePath;
  inDir = IOUtil::CreateTemporaryDirectory("monai-in-XXXXXX", this->GetMitkTempDir());
  std::ofstream tmpStream;
  inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, m_TEMPLATE_FILENAME, inDir + IOUtil::GetDirectorySeparator());
  tmpStream.close();
  std::size_t found = inputImagePath.find_last_of(IOUtil::GetDirectorySeparator());
  std::string fileName = inputImagePath.substr(found + 1);
  std::string token = fileName.substr(0, fileName.find("_"));
  outDir = IOUtil::CreateTemporaryDirectory("monai-out-XXXXXX", this->GetMitkTempDir());
  outputImagePath = outDir + IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";

  try
  {
    m_IsLastSuccess = false;
    if (!m_TEST)
    {
      IOUtil::Save(inputAtTimeStep, inputImagePath);
      PostInferRequest(hostName, port, inputImagePath, outputImagePath);
    }
    else
    {
      std::string metaData_test =
        "{\"label_names\":{\"spleen\": 1, \"right kidney\": 2, \"left kidney\": 3, \"liver\": 6, \"stomach\": 7, "
        "\"aorta\": 8, \"inferior "
        "vena cava\": 9, \"background\": 0}, \"latencies\": {\"pre\": 0.64, \"infer\": 1.13, \"invert\": 0.03, "
        "\"post\": 0.06, \"write\": 0.1, \"total\": 1.96, \"transform\": {\"pre\": {\"LoadImaged\": 0.1483, "
        "\"EnsureChannelFirstd\": 0.0, \"Orientationd\": 0.0, \"ScaleIntensityRanged\": 0.0403, \"Resized\": 0.0401, "
        "\"DiscardAddGuidanced\": 0.0297, \"EnsureTyped\": 0.3703}, \"post\": {\"EnsureTyped\": 0.0, \"Activationsd\": "
        "0.0, \"AsDiscreted\": 0.0, \"SqueezeDimd\": 0.0, \"ToNumpyd\": 0.0473, \"Restored\": 0.0001}}}}";
      m_ResultMetadata = nlohmann::json::parse(metaData_test);
      outputImagePath = "C:/DKFZ/MONAI_work/monai_test_python/output.nii.gz";
    }

    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    auto outputBuffer = mitk::LabelSetImage::New();
    outputBuffer->InitializeByLabeledImage(outputImage);
    outputBuffer->SetGeometry(inputAtTimeStep->GetGeometry());

    std::map<std::string, mitk::Label::PixelType> labelMap = m_ResultMetadata["label_names"];
    MapLabelsToSegmentation(outputBuffer, previewImage, labelMap);
    mitk::ImageReadAccessor newMitkImgAcc(outputBuffer.GetPointer());
    previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
    this->SetIsLastSuccess(true);
  }
  catch (const mitk::Exception &e)
  {
    m_IsLastSuccess = false;
    MITK_ERROR << e.GetDescription();
    mitkThrow() << e.GetDescription();
  }
}

void mitk::MonaiLabelTool::MapLabelsToSegmentation(const mitk::LabelSetImage *source,
                                                   mitk::LabelSetImage *dest,
                                                   std::map<std::string, mitk::Label::PixelType> &labelMap)
{
  std::map<mitk::Label::PixelType, std::string> flippedLabelMap;
  for (auto const &[key, val] : labelMap)
  {
    flippedLabelMap[val] = key;
  }
  auto labelset = dest->GetLabelSet();
  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(mitk::LookupTable::LookupTableType::MULTILABEL);
  mitk::Label::PixelType labelId = 0;
  for (auto const &[key, val] : flippedLabelMap)
  {
    if (source->ExistLabel(labelId, source->GetActiveLayer()))
    {
      Label::Pointer label = Label::New(labelId, val);
      std::array<double, 3> lookupTableColor;
      lookupTable->GetColor(labelId, lookupTableColor.data());
      Color color;
      color.SetRed(lookupTableColor[0]);
      color.SetGreen(lookupTableColor[1]);
      color.SetBlue(lookupTableColor[2]);
      label->SetColor(color);
      labelset->AddLabel(label, false);
    }
    else
    {
      MITK_INFO << "Label not found for " << val;
    }
    labelId++;
  }
}

void mitk::MonaiLabelTool::GetOverallInfo(std::string &hostName, int &port)
{
  MITK_INFO << "URL..." << hostName << ":" << port;
  if (!IsMonaiServerOn(hostName, port))
  {
    Tool::ErrorMessage.Send(m_SERVER_503_ERROR_TEXT);
    mitkThrow() << m_SERVER_503_ERROR_TEXT;
  }
  httplib::Client cli(hostName, port); // httplib::Client cli("localhost", 8000);
  if (auto response = cli.Get("/info/"))
  {
    if (response->status == 200)
    {
      auto jsonObj = nlohmann::json::parse(response->body);
      if (jsonObj.is_discarded() || !jsonObj.is_object())
      {
        MITK_ERROR << "Could not parse \"" << /* jsonPath.toStdString() << */ "\" as JSON object!";
        return;
      }
      m_InfoParameters = DataMapper(jsonObj);
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

void mitk::MonaiLabelTool::PostInferRequest(std::string &hostName,
                                            int &port,
                                            std::string &filePath,
                                            std::string &outFile)
{
  // std::string url = "http://localhost:8000/infer/deepedit_seg"; // + m_ModelName;
  std::string &modelName = m_RequestParameters->model.name; // Get this from args as well.
  std::string postPath = "/infer/";                         // make this separate class of constants
  postPath.append(modelName);

  // std::string filePath = "C:/DKFZ/MONAI_work/monai_test_python/la_030.nii.gz";
  std::ifstream input(filePath, std::ios::binary);
  if (!input)
  {
    MITK_WARN << "could not read file to POST";
  }
  std::stringstream buffer_lf_img;
  buffer_lf_img << input.rdbuf();
  input.close();

  httplib::MultipartFormDataItems items = {
    {"file", buffer_lf_img.str(), "post_from_mitk.nii.gz", "application/octet-stream"}};
  // httplib::MultipartFormDataMap itemMap = {{"file", {"file", buffer_lf_img.str(), "spleen_58.nii.gz",
  // "application/octet-stream"}}};

  httplib::Client cli(hostName, port);
  cli.set_read_timeout(60);                      // arbitary 1 minute time-out to avoid corner cases.
  if (auto response = cli.Post(postPath, items)) // cli.Post("/infer/deepedit_seg", items);
  {
    if (response->status == 200)
    {
      // Find boundary
      httplib::Headers headers = response->headers;
      std::string contentType = headers.find("content-type")->second;
      std::string delimiter = "boundary=";
      std::string boundaryString =
        contentType.substr(contentType.find(delimiter) + delimiter.length(), std::string::npos);
      boundaryString.insert(0, "--");
      MITK_INFO << "boundary hash: " << boundaryString;

      // Parse metadata JSON
      std::string resBody = response->body;
      std::vector<std::string> multiPartResponse = this->getPartsBetweenBoundary(resBody, boundaryString);

      std::string metaData = multiPartResponse[0];
      std::string contentTypeJson = "Content-Type: application/json";
      size_t ctPos = metaData.find(contentTypeJson) + contentTypeJson.length();
      std::string metaDataPart = metaData.substr(ctPos);
      MITK_INFO << metaDataPart;
      auto jsonObj = nlohmann::json::parse(metaDataPart);
      if (jsonObj.is_discarded() || !jsonObj.is_object())
      {
        MITK_ERROR << "Could not parse \"" << /* jsonPath.toStdString() << */ "\" as JSON object!";
        return;
      }
      else // use the metadata
      {
        m_ResultMetadata = jsonObj;
      }
      // Parse response image string
      std::string imagePart = multiPartResponse[1];
      std::string contentTypeStream = "Content-Type: application/octet-stream";
      ctPos = imagePart.find(contentTypeStream) + contentTypeStream.length();
      std::string imageDataPart = imagePart.substr(ctPos);

      MITK_INFO << imageDataPart.substr(0, 50);
      std::string whitespaces = " \n\r";
      imageDataPart.erase(0, imageDataPart.find_first_not_of(whitespaces)); // clean up
      std::ofstream output(outFile, std::ios::out | std::ios::app | std::ios::binary);
      output << imageDataPart;
      output.unsetf(std::ios::skipws);
      output.flush();
      output.close(); // necessary to close? RAII
    }
    else
    {
      auto err = response.error();
      MITK_ERROR << "An HTTP POST error: " << httplib::to_string(err) << " occured.";
      mitkThrow() << "An HTTP POST error: " << httplib::to_string(err) << " occured.";
    }
  }
}

std::vector<std::string> mitk::MonaiLabelTool::getPartsBetweenBoundary(const std::string &body,
                                                                       const std::string &boundary)
{
  std::vector<std::string> retVal;
  std::string master = body;
  size_t found = master.find(boundary);
  size_t beginPos = 0;
  while (found != std::string::npos)
  {
    std::string part = master.substr(beginPos, found);
    if (!part.empty())
    {
      retVal.push_back(part);
    }
    master.erase(beginPos, found + boundary.length());
    found = master.find(boundary);
  }
  return retVal;
}

std::unique_ptr<mitk::MonaiAppMetadata> mitk::MonaiLabelTool::DataMapper(nlohmann::json &jsonObj)
{
  auto object = std::make_unique<MonaiAppMetadata>();
  object->name = jsonObj["name"].get<std::string>();
  object->description = jsonObj["description"].get<std::string>();
  object->labels = jsonObj["labels"].get<std::vector<std::string>>();

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
    MITK_INFO << "DIMENSION: " << modelInfo.dimension;
    modelInfo.description = _jsonObj["description"].get<std::string>();

    object->models.push_back(modelInfo);
  }
  return object;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetAutoSegmentationModels(int dim)
{
  std::vector<mitk::MonaiModelInfo> autoModels;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_AUTO_SEG_TYPE_NAME.find(model.type) != m_AUTO_SEG_TYPE_NAME.end() && model.dimension == dim)
      {
        autoModels.push_back(model);
      }
    }
  }
  return autoModels;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetInteractiveSegmentationModels(int dim)
{
  std::vector<mitk::MonaiModelInfo> interactiveModels;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_INTERACTIVE_SEG_TYPE_NAME.find(model.type) != m_INTERACTIVE_SEG_TYPE_NAME.end() && model.dimension == dim)
      {
        interactiveModels.push_back(model);
      }
    }
  }
  return interactiveModels;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetScribbleSegmentationModels(int dim)
{
  std::vector<mitk::MonaiModelInfo> scribbleModels;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_SCRIBBLE_SEG_TYPE_NAME.find(model.type) != m_SCRIBBLE_SEG_TYPE_NAME.end() && model.dimension == dim)
      {
        scribbleModels.push_back(model);
      }
    }
  }
  return scribbleModels;
}
