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

#include "SegmentationReworkREST.h"
#include <mitkCommon.h>
#include <mitkRESTUtil.h>

SegmentationReworkREST::SegmentationReworkREST() {}

SegmentationReworkREST::SegmentationReworkREST(utility::string_t url) : mitk::RESTServer(url)
{
  m_Listener.support(MitkRESTMethods::PUT, std::bind(&SegmentationReworkREST::HandlePut, this, std::placeholders::_1));
  m_Listener.support(MitkRESTMethods::GET, std::bind(&SegmentationReworkREST::HandleGet, this, std::placeholders::_1));
}

SegmentationReworkREST::~SegmentationReworkREST() {}

void SegmentationReworkREST::HandleGet(MitkRequest message) 
{
  auto messageString = message.to_string();
  MITK_INFO << "Message GET incoming...";
  MITK_INFO << mitk::RESTUtil::convertToUtf8(messageString);

  auto httpParams = web::uri::split_query(message.request_uri().query());

  // IHE Invoke Image Display style
  auto requestType = httpParams.find(L"requestType");

  if (requestType != httpParams.end() && requestType->second == L"IMAGE_SEG")
  {
    try
    {
      auto studyUID = httpParams.at(L"studyUID");
      auto imageSeriesUID = httpParams.at(L"imageSeriesUID");
      auto segSeriesUID = httpParams.at(L"segSeriesUID");

      DicomDTO dto;
      dto.imageSeriesUID = mitk::RESTUtil::convertToUtf8(imageSeriesUID);
      dto.studyUID = mitk::RESTUtil::convertToUtf8(studyUID);
      dto.segSeriesUIDA = mitk::RESTUtil::convertToUtf8(segSeriesUID);

      m_GetImageSegCallback(dto);

      MitkResponse response(MitkRestStatusCodes::OK);
      response.set_body("Sure, i got you.. have an awesome day");
      message.reply(response);
    }
    catch (std::out_of_range& e) {
      message.reply(MitkRestStatusCodes::BadRequest, "oh man, that was not expected: " + std::string(e.what()));
    }
  }
  else if (requestType != httpParams.end() && requestType->second == L"SEG_EVALUATION") 
  {
    // TODO: implement PUT handling as GET
  } else
  {
    message.reply(MitkRestStatusCodes::BadRequest, "Oh man, i can only deal with 'requestType' = 'IMAGE+SEG'...");
  }
}

void SegmentationReworkREST::HandlePut(MitkRequest message)
{
  auto messageString = message.to_string();
  MITK_INFO << "Message PUT incoming...";
  MITK_INFO << mitk::RESTUtil::convertToUtf8(messageString);

  try
  {
    auto jsonMessage = message.extract_json().get();

    auto messageTypeKey = jsonMessage.at(U("messageType"));
    if (messageTypeKey.as_string() == U("downloadData"))
    {
      auto imageStudyUIDKey = jsonMessage.at(U("studyUID"));
      auto srSeriesUIDKey = jsonMessage.at(U("srSeriesUID"));

      auto groundTruthKey = jsonMessage.at(U("groundTruth"));

      auto simScoreKey = jsonMessage.at(U("simScoreArray"));
      auto minSliceStartKey = jsonMessage.at(U("minSliceStart"));

      DicomDTO dto;
      dto.srSeriesUID = mitk::RESTUtil::convertToUtf8(srSeriesUIDKey.as_string());

      dto.groundTruth = mitk::RESTUtil::convertToUtf8(groundTruthKey.as_string());

      dto.studyUID = mitk::RESTUtil::convertToUtf8(imageStudyUIDKey.as_string());
      dto.minSliceStart = minSliceStartKey.as_integer();

      std::vector<double> vec;
      web::json::array simArray = simScoreKey.as_array();

      for (web::json::value score : simArray)
      {
        vec.push_back(score.as_double());
      }

      dto.simScoreArray = vec;
      m_PutCallback(dto);
      emit InvokeUpdateChartWidget();
    }
    else
    {
      message.reply(MitkRestStatusCodes::BadRequest, "Oh man, i can only deal with 'messageType' = 'downloadData'...");
    }
  }
  catch (MitkJsonException &e)
  {
    MITK_ERROR << e.what() << ".. oh man, that was not expected";
    message.reply(MitkRestStatusCodes::BadRequest, "oh man, that was not expected");
    return;
  }
  MitkResponse response(MitkRestStatusCodes::OK);
  response.headers().add(U("Access-Control-Allow-Origin"), U("localhost:9000/*"));
  response.set_body("Sure, i got you.. have an awesome day");
  message.reply(response);
  return;
}