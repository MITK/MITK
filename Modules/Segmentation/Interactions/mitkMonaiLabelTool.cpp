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
#include "mitkInteractionPositionEvent.h"
#include "mitkPointSetShapeProperty.h"
#include "mitkProperties.h"
#include "mitkToolManager.h"

mitk::MonaiLabelTool::MonaiLabelTool() : SegWithPreviewTool(true, "PressMoveReleaseAndPointSetting")
{
  this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
}

mitk::MonaiLabelTool::~MonaiLabelTool()
{
  itksys::SystemTools::RemoveADirectory(this->GetMitkTempDir());
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

  this->SetLabelTransferScope(LabelTransferScope::ActiveLabel);
  this->SetLabelTransferMode(LabelTransferMode::MapLabel);
}

void mitk::MonaiLabelTool::Deactivated()
{
  this->ClearSeeds();
  GetDataStorage()->Remove(m_PointSetNodePositive);
  GetDataStorage()->Remove(m_PointSetNodeNegative);
  m_PointSetNodePositive = nullptr;
  m_PointSetNodeNegative = nullptr;
  m_PointSetPositive = nullptr;
  m_PointSetNegative = nullptr;
  Superclass::Deactivated();
}

void mitk::MonaiLabelTool::OnAddPositivePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if ("deepgrow" == m_RequestParameters->model.type || "deepedit" == m_RequestParameters->model.type)
  {
    /* if ((nullptr == this->GetWorkingPlaneGeometry()) ||
        !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()),
                     *(this->GetWorkingPlaneGeometry())))
    {
      this->ClearSeeds();
      this->SetWorkingPlaneGeometry(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone());
    }*/
    if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
    {
      const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
      if (positionEvent != nullptr)
      {
        m_PointSetPositive->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
        ++m_PointSetCount;
        this->UpdatePreview();
      }
    }
  }
}

void mitk::MonaiLabelTool::OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if ("deepgrow" != m_RequestParameters->model.type)
  {
    return;
  }
  /* if (m_PointSetPositive->GetSize() == 0 || nullptr == this->GetWorkingPlaneGeometry() ||
      !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()),
                   *(this->GetWorkingPlaneGeometry())))
  {
    return;
  }*/
  if (!this->IsUpdating() && m_PointSetNegative.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent != nullptr)
    {
      m_PointSetNegative->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
      m_PointSetCount++;
      this->UpdatePreview();
    }
  }
}

void mitk::MonaiLabelTool::OnDelete(StateMachineAction *, InteractionEvent *)
{
  if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
  {
    PointSet::Pointer removeSet = m_PointSetPositive;
    decltype(m_PointSetPositive->GetMaxId().Index()) maxId = 0;
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
      PostInferRequest(hostName, port, inputImagePath, outputImagePath, inputAtTimeStep->GetGeometry());
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

    if (m_AUTO_SEG_TYPE_NAME.find(m_RequestParameters->model.type) != m_AUTO_SEG_TYPE_NAME.end())
    {
      std::map<std::string, mitk::Label::PixelType> labelMap = m_ResultMetadata["label_names"];
      this->MapLabelsToSegmentation(outputBuffer, previewImage, labelMap);
    }
    else
    {
      std::map<std::string, mitk::Label::PixelType> labelMap{{m_RequestParameters->requestLabel, 1}};
      this->MapLabelsToSegmentation(outputBuffer, previewImage, labelMap);
    }
    mitk::ImageReadAccessor newMitkImgAcc(outputBuffer.GetPointer());
    previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
    this->SetIsLastSuccess(true);
    MonaiStatusEvent.Send(true);
  }
  catch (const mitk::Exception &e)
  {
    m_IsLastSuccess = false;
    MITK_ERROR << e.GetDescription();
    mitkThrow() << e.GetDescription();
    MonaiStatusEvent.Send(false);
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
  //mitk::Label::PixelType labelId = 0;
  for (auto const &[key, val] : flippedLabelMap)
  {
    //if (source->ExistLabel(labelId, source->GetActiveLayer()))
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
      labelset->AddLabel(label, false);
    }
    else
    {
      MITK_INFO << "Label not found for " << val;
    }
    // labelId++;
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
                                            std::string &outFile, 
                                            const mitk::BaseGeometry *baseGeometry)
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
  httplib::MultipartFormDataItems items;
  if (m_INTERACTIVE_SEG_TYPE_NAME.find(m_RequestParameters->model.type) != m_INTERACTIVE_SEG_TYPE_NAME.end())
  {
    std::stringstream foreground = this->GetPointsAsListString(baseGeometry, m_PointSetPositive);
    std::stringstream background = this->GetPointsAsListString(baseGeometry, m_PointSetNegative);
    std::stringstream paramString;
    paramString << "{\"foreground\":" << foreground.str() << ",\"background\":" << background.str() << "}";
    MITK_INFO << foreground.str(); MITK_INFO << background.str(); MITK_INFO << paramString.str();
    items.push_back({"params", paramString.str()});
  }
  //httplib::MultipartFormDataItems items = {
    //{"params", "{\"restore_label_idx\": true} "},
    //{"params", "{\"foreground\" : [[ 59, 43, 56 ]], \"background\" : []}"},
    //{"file", buffer_lf_img.str(), "post_from_mitk.nii.gz", "application/octet-stream"}};
  items.push_back({"file", buffer_lf_img.str(), "post_from_mitk.nii.gz", "application/octet-stream"});
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
    modelInfo.description = _jsonObj["description"].get<std::string>();

    object->models.push_back(modelInfo);
  }
  return object;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetAutoSegmentationModels(int dim)
{
  std::vector<mitk::MonaiModelInfo> autoModels;
  bool checkDims = dim > -1;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_AUTO_SEG_TYPE_NAME.find(model.type) != m_AUTO_SEG_TYPE_NAME.end()
          && (!checkDims || model.dimension == dim))
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
  bool checkDims = dim > -1;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_INTERACTIVE_SEG_TYPE_NAME.find(model.type) != m_INTERACTIVE_SEG_TYPE_NAME.end()
          && (!checkDims || model.dimension == dim))
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
  bool checkDims = dim > -1;
  if (nullptr != m_InfoParameters)
  {
    for (mitk::MonaiModelInfo &model : m_InfoParameters->models)
    {
      if (m_SCRIBBLE_SEG_TYPE_NAME.find(model.type) != m_SCRIBBLE_SEG_TYPE_NAME.end()
          && (!checkDims || model.dimension == dim))
      {
        scribbleModels.push_back(model);
      }
    }
  }
  return scribbleModels;
}

mitk::MonaiModelInfo mitk::MonaiLabelTool::GetModelInfoFromName(std::string &modelName)
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

std::stringstream mitk::MonaiLabelTool::GetPointsAsListString(const mitk::BaseGeometry *baseGeometry,
                                                              PointSet::Pointer pointSet)
{
  MITK_INFO << "No.of points: " << pointSet->GetSize();
  std::stringstream pointsAndLabels;
  pointsAndLabels << "[";
  mitk::PointSet::PointsConstIterator pointSetItPos = pointSet->Begin();
  const char COMMA = ',';
  while (pointSetItPos != pointSet->End())
  {
    mitk::Point3D point3d = pointSetItPos.Value();
    if (baseGeometry->IsInside(point3d))
    {
      mitk::Point3D index3D;
      baseGeometry->WorldToIndex(point3d, index3D);
      pointsAndLabels << "[";
      pointsAndLabels << static_cast<int>(index3D[0]) << COMMA << static_cast<int>(index3D[1]) << COMMA
                      << static_cast<int>(index3D[2]) << "],";
    }
    ++pointSetItPos;
  }
  if (pointsAndLabels.tellp() > 1)
  {
    pointsAndLabels.seekp(-1, pointsAndLabels.end); // remove last added comma character
  }
  pointsAndLabels << "]";
  return pointsAndLabels;
}
