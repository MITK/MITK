#include "SegmentationReworkREST.h"

#include <mitkCommon.h>

SegmentationReworkREST::SegmentationReworkREST(utility::string_t url) : mitk::RESTServer(url) 
{
  m_Listener.support(MitkRESTMethods::PUT, std::bind(&SegmentationReworkREST::HandlePut, this, std::placeholders::_1));
}

SegmentationReworkREST::~SegmentationReworkREST() 
{
}

void SegmentationReworkREST::HandlePut(MitkRequest message)
{
  MITK_DEBUG << utility::conversions::to_utf8string(message.to_string());

  message.reply(MitkRestStatusCodes::OK, "Sure, i got you.. have an awesome day");
  return;
}
