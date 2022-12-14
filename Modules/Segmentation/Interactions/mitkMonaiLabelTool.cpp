/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkMonaiLabelTool.h"

// us
#include "mitkIOUtil.h"
#include "mitkRESTUtil.h"
#include <map>
#include <nlohmann/json.hpp>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>
#include <vector>
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabelTool, "MonaiLabel");
}

mitk::MonaiLabelTool::MonaiLabelTool()
{
  InitializeRESTManager();
}

void mitk::MonaiLabelTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferMode(LabelTransferMode::AllLabels);
}

const char **mitk::MonaiLabelTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::MonaiLabelTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu_48x48.png");
  return resource;
}

const char *mitk::MonaiLabelTool::GetName() const
{
  return "MonaiLabel";
}

void mitk::MonaiLabelTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                           const Image * /*oldSegAtTimeStep*/,
                                           LabelSetImage *previewImage,
                                           TimeStepType timeStep)
{
  std::string outputImagePath = "Z:/dataset/Task05_Prostate/labelsTr/prostate_00.nii.gz";
  try
  {
    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    previewImage->InitializeByLabeledImage(outputImage);
    previewImage->SetGeometry(inputAtTimeStep->GetGeometry());
  }
  catch (const mitk::Exception &e)
  {
    /*
    Can't throw mitk exception to the caller. Refer: T28691
    */
    MITK_ERROR << e.GetDescription();
    return;
  }
}

void mitk::MonaiLabelTool::InitializeRESTManager() // Don't call from constructor --ashis
{
  auto serviceRef = us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();
  if (serviceRef)
  {
    m_RESTManager = us::GetModuleContext()->GetService(serviceRef);
  }
}

void mitk::MonaiLabelTool::GetOverallInfo(std::string url)
{
  if (m_RESTManager != nullptr)
  {
    PostSegmentationRequest();
    std::string jsonString;
    bool fetched = false;
    web::json::value result;
    m_RESTManager->SendRequest(mitk::RESTUtil::convertToTString(url))
      .then(
        [&](pplx::task<web::json::value> resultTask) /*It is important to use task-based continuation*/
        {
          try
          {
            result = resultTask.get();
            fetched = true;
          }
          catch (const mitk::Exception &exception)
          {
            MITK_ERROR << exception.what();
            return;
          }
        })
      .wait();
    if (fetched)
    {
      m_Parameters = DataMapper(result);
    }
  }
}

std::unique_ptr<mitk::MonaiAppMetadata> mitk::MonaiLabelTool::DataMapper(web::json::value &result)
{
  auto object = std::make_unique<MonaiAppMetadata>();
  utility::string_t stringT = result.to_string();
  std::string jsonString(stringT.begin(), stringT.end());
  auto jsonObj = nlohmann::json::parse(jsonString);
  if (jsonObj.is_discarded() || !jsonObj.is_object())
  {
    MITK_ERROR << "Could not parse \"" << jsonString << "\" as JSON object!";
  }
  //MITK_INFO << jsonString;
  MITK_INFO << "ashis inside mapper " << jsonObj["name"].get<std::string>(); // remove
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


std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetAutoSegmentationModels() 
{
  std::vector<mitk::MonaiModelInfo> autoModels;
  if (nullptr != m_Parameters)
  {
    for (mitk::MonaiModelInfo &model : m_Parameters->models)
    {
      if (m_AUTO_SEG_TYPE_NAME.find(model.type) != m_AUTO_SEG_TYPE_NAME.end())
      {
        autoModels.push_back(model);
      }
    }
  }
  return autoModels;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetInteractiveSegmentationModels()
{
  std::vector<mitk::MonaiModelInfo> interactiveModels;
  if (nullptr != m_Parameters)
  {
    for (mitk::MonaiModelInfo &model : m_Parameters->models)
    {
      if (m_INTERACTIVE_SEG_TYPE_NAME.find(model.type) != m_INTERACTIVE_SEG_TYPE_NAME.end())
      {
        interactiveModels.push_back(model);
      }
    }
  }
  return interactiveModels;
}

std::vector<mitk::MonaiModelInfo> mitk::MonaiLabelTool::GetScribbleSegmentationModels()
{
  std::vector<mitk::MonaiModelInfo> scribbleModels;
  if (nullptr != m_Parameters)
  {
    for (mitk::MonaiModelInfo &model : m_Parameters->models)
    {
      if (m_SCRIBBLE_SEG_TYPE_NAME.find(model.type) != m_SCRIBBLE_SEG_TYPE_NAME.end())
      {
        scribbleModels.push_back(model);
      }
    }
  }
  return scribbleModels;
}

void mitk::MonaiLabelTool::PostSegmentationRequest() // return LabelSetImage ??
{
  // web::json::value result;
  // web::json::value data;

  std::string url = "http://localhost:8000/infer/deepedit_seg"; // + m_ModelName;
  std::string filePath = "//vmware-host//Shared Folders//Downloads//spleen_58.nii.gz";
  std::ifstream input(filePath, std::ios::binary);
  if (!input)
  {
    MITK_WARN << "could not read file to POST";
  }

  std::vector<unsigned char> result;
  std::vector<unsigned char> buffer;

  // Stop eating new lines in binary mode!!!
  input.unsetf(std::ios::skipws);

  input.seekg(0, std::ios::end);
  const std::streampos fileSize = input.tellg();
  input.seekg(0, std::ios::beg);

  MITK_INFO << fileSize << " bytes will be sent.";
  buffer.reserve(fileSize); // file size
  std::copy(
    std::istream_iterator<unsigned char>(input), std::istream_iterator<unsigned char>(), std::back_inserter(buffer));


  mitk::RESTUtil::ParamMap headers;
  headers.insert(mitk::RESTUtil::ParamMap::value_type(U("accept"), U("application/json")));
  // headers.insert(mitk::RESTUtil::ParamMap::value_type(U("Accept-Encoding"), U("gzip, deflate")));
  headers.insert(mitk::RESTUtil::ParamMap::value_type(
    // U("Content-Type"), U("multipart/related; type=\"application/dicom\"; boundary=boundary")));
    U("Content-Type"), U("multipart/form-data; boundary=boundary")));

  // in future more than one file should also be supported..
  std::string head = "";
  head += "\r\n--boundary";
  head += "\r\nContent-Disposition: form-data; name=\"params\"\r\n\r\n{}";
  head += "\r\n--boundary";
  head += "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"spleen_58.nii.gz\"\r\n\r\n";

  std::vector<unsigned char> bodyVector(head.begin(), head.end());

  std::string tail = "";
  tail += "\r\n--boundary";
  tail += "\r\nContent-Disposition: form-data; name=\"label\"\r\n\r\n";
  tail += "\r\n--boundary--\r\n";

  result.insert(result.end(), bodyVector.begin(), bodyVector.end());
  result.insert(result.end(), buffer.begin(), buffer.end());
  result.insert(result.end(), tail.begin(), tail.end());

  try
  {
    m_RESTManager
      ->SendBinaryRequest(mitk::RESTUtil::convertToTString(url), mitk::IRESTManager::RequestType::Post, &result, headers)
      .then(
        [=](pplx::task<web::json::value> result) /* (web::json::value result)*/
        {
          MITK_INFO << "after send";
          //MITK_INFO << mitk::RESTUtil::convertToUtf8(result.serialize());
          //result.is_null();
        })
      .wait();
  }
  catch (std::exception &e)
  {
    MITK_WARN << e.what();
  }
}