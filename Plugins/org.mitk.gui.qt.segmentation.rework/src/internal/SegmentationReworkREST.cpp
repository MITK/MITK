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

SegmentationReworkREST::SegmentationReworkREST() {}

SegmentationReworkREST::SegmentationReworkREST(utility::string_t url) : mitk::RESTServer(url)
{
  m_Listener.support(MitkRESTMethods::PUT, std::bind(&SegmentationReworkREST::HandlePut, this, std::placeholders::_1));
}

SegmentationReworkREST::~SegmentationReworkREST() {}

void SegmentationReworkREST::SetPutCallback(std::function<void(DicomDTO& message)> callback)
{
  m_PutCallback = callback;
}

void SegmentationReworkREST::HandlePut(MitkRequest message)
{
  auto messageString = message.to_string();
  MITK_INFO << "Message PUT incoming...";
  MITK_INFO << convertToUtf8(messageString);

  try
  {
    auto jsonMessage = message.extract_json().get();

    auto messageTypeKey = jsonMessage.at(U("messageType"));
    if (messageTypeKey.as_string() == U("downloadData"))
    {
      auto imageStudyUIDKey = jsonMessage.at(U("studyUID"));
      auto imageSeriesUIDKey = jsonMessage.at(U("imageSeriesUID"));
      auto segSeriesUIDAKey = jsonMessage.at(U("segSeriesUIDA"));
      auto segSeriesUIDBKey = jsonMessage.at(U("segSeriesUIDB"));

	  auto segInstanceUIDAKey = jsonMessage.at(U("segInstanceUIDA"));
      auto segInstanceUIDBKey = jsonMessage.at(U("segInstanceUIDB"));

      auto simScoreKey = jsonMessage.at(U("simScoreArray"));
      auto minSliceStartKey = jsonMessage.at(U("minSliceStart"));

      DicomDTO dto;
      dto.segSeriesUIDA = convertToUtf8(segSeriesUIDAKey.as_string());
      dto.segSeriesUIDB = convertToUtf8(segSeriesUIDBKey.as_string());
      dto.imageSeriesUID = convertToUtf8(imageSeriesUIDKey.as_string());
      dto.segInstanceUIDA = convertToUtf8(segInstanceUIDAKey.as_string());
      dto.segInstanceUIDB = convertToUtf8(segInstanceUIDBKey.as_string());

      dto.studyUID = convertToUtf8(imageStudyUIDKey.as_string());
      dto.minSliceStart = minSliceStartKey.as_integer();

      std::vector<double> vec;
      web::json::array simArray = simScoreKey.as_array();

      for (web::json::value score : simArray) {
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