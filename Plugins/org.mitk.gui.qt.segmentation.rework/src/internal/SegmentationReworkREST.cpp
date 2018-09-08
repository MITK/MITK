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

SegmentationReworkREST::SegmentationReworkREST(utility::string_t url) : mitk::RESTServer(url)
{
  m_Listener.support(MitkRESTMethods::PUT, std::bind(&SegmentationReworkREST::HandlePut, this, std::placeholders::_1));
}

SegmentationReworkREST::~SegmentationReworkREST() {}

void SegmentationReworkREST::HandlePut(MitkRequest message)
{
  auto messageString = message.to_string();
  MITK_INFO << "Message PUT incoming...";
  MITK_INFO << convertToUtf8(messageString);

  std::string imageSeriesUID = "";
  try
  {
    auto jsonMessage = message.extract_json().get();

    auto messageTypeKey = jsonMessage.at(U("messageType"));
    if (messageTypeKey.as_string() == U("downloadData"))
    {
      auto imageSeriesUIDKey = jsonMessage.at(U("imageSeriesUID"));
      imageSeriesUID = convertToUtf8(imageSeriesUIDKey.as_string());
      MITK_INFO << "value of imageSeriesUID " << imageSeriesUID;
      // trigger download function
    }
    else
    {
      message.reply(MitkRestStatusCodes::BadRequest, "Oh man, i can only deal with 'messageType' = 'downloadData'...");
    }
  }
  catch (MitkJsonException &e)
  {
    MITK_ERROR << e.what() << ".. oh man, that was not expected";
  }
  message.reply(MitkRestStatusCodes::OK, "Sure, i got you.. have an awesome day");
  return;
}
