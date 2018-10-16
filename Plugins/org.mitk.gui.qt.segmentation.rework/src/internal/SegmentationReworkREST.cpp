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

#include <sstream>
#include <iostream>

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

  MITK_INFO << mitk::RESTUtil::convertToUtf8(message.request_uri().to_string());
  auto uri = web::uri::decode(message.request_uri().to_string());

  if (uri.find(U("/robots.txt")) != std::string::npos || uri.find(U("/favicon.ico")) != std::string::npos || uri == U("/-") || uri.length() == 0)
  {
    MITK_INFO << "ignore GET request.";
    return;
  }

  auto query = web::uri(uri).query();
  auto httpParams = web::uri::split_query(query);

  // IHE Invoke Image Display style
  auto requestType = httpParams.find(U("requestType"));
  MITK_INFO << "parameters found: " << httpParams.size();

  MitkResponse responsePositive(MitkRestStatusCodes::OK);
  responsePositive.set_body("call to mitk was successfully!");

  if (requestType != httpParams.end() && requestType->second == U("IMAGE_SEG"))
  {
    try
    {
      auto studyUID = httpParams.at(U("studyUID"));
      auto imageSeriesUID = httpParams.at(U("imageSeriesUID"));
      auto segSeriesUID = httpParams.at(U("segSeriesUID"));

      DicomDTO dto;
      dto.imageSeriesUID = mitk::RESTUtil::convertToUtf8(imageSeriesUID);
      dto.studyUID = mitk::RESTUtil::convertToUtf8(studyUID);
      dto.segSeriesUIDA = mitk::RESTUtil::convertToUtf8(segSeriesUID);

      MITK_INFO << "studyUID: " << dto.studyUID;
      MITK_INFO << "imageSeriesUID: " << dto.imageSeriesUID;
      MITK_INFO << "segSeriesUID: " << dto.segSeriesUIDA;

      m_GetImageSegCallback(dto);

      MitkResponse response(MitkRestStatusCodes::OK);
      message.reply(responsePositive);
    }
    catch (std::out_of_range& e) {
      message.reply(MitkRestStatusCodes::BadRequest, "oh man, that was not expected: " + std::string(e.what()));
    }
  } else if (requestType != httpParams.end() && requestType->second == U("SEG_EVALUATION")) 
  {
    // TODO: implement PUT handling as GET
  } else if (requestType != httpParams.end() && requestType->second == U("IMAGE_SEG_LIST"))
  {
    auto studyUID = httpParams.at(U("studyUID"));
    auto imageSeriesUID = httpParams.at(U("imageSeriesUID"));
    auto segSeriesUIDList = httpParams.at(U("segSeriesUIDList"));

    DicomDTO dto;
    dto.imageSeriesUID = mitk::RESTUtil::convertToUtf8(imageSeriesUID);
    dto.studyUID = mitk::RESTUtil::convertToUtf8(studyUID);

    auto segSeriesUIDListUtf8 = mitk::RESTUtil::convertToUtf8(segSeriesUIDList);
  
    std::istringstream f(segSeriesUIDListUtf8);
    std::string s;
    while (getline(f, s, ',')) {
      dto.seriesUIDList.push_back(s);
    }

    MITK_INFO << "studyUID: " << dto.studyUID;
    MITK_INFO << "imageSeriesUID: " << dto.imageSeriesUID;
    MITK_INFO << "segSeriesUIDList: " << segSeriesUIDListUtf8;

    m_GetImageSegCallback(dto);

    message.reply(responsePositive);
  } else if (requestType != httpParams.end() && requestType->second == U("ADD_SERIES"))
  {
    auto studyUID = httpParams.at(U("studyUID"));
    auto segSeriesUIDList = httpParams.at(U("seriesUIDList"));

    DicomDTO dto;
    dto.studyUID = mitk::RESTUtil::convertToUtf8(studyUID);

    auto seriesUIDListUtf8 = mitk::RESTUtil::convertToUtf8(segSeriesUIDList);

    std::istringstream f(seriesUIDListUtf8);
    std::string s;
    while (getline(f, s, ',')) {
      dto.seriesUIDList.push_back(s);
    }

    MITK_INFO << "studyUID: " << dto.studyUID;
    MITK_INFO << "seriesUIDList: " << seriesUIDListUtf8;

    m_GetAddSeriesCallback(dto);
    message.reply(responsePositive);
  } else
  {
    message.reply(MitkRestStatusCodes::BadRequest, "unknown request type " + mitk::RESTUtil::convertToUtf8(requestType->second));
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
        vec.push_back(score.as_double() * 100);
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