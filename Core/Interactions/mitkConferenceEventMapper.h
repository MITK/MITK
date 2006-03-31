#ifndef CONFERENCEVENTMAPPER_H_HEADER_INCLUDE
#define CONFERENCEVENTMAPPER_H_HEADER_INCLUDE

#include <mitkEventMapper.h>

namespace mitk {

  class ConferenceEventMapper : public EventMapper
  {
    public:
      ConferenceEventMapper();

      ~ConferenceEventMapper();

      // First part MITK Event ID and baserendere name
      // Second the normaly not needed integer information from the GUI Event (type, state, button, key)
      // Third the koordinated 3D and the relativ 2D
      static bool MapEvent(signed int id, const char* sender, int Etype, int Estate, int EButtonState, int key, float w1,float w2,float w3,float d0,float d1);

      /* EasyEvent for MousMove Interaction.
       * used during conferences
       */
      static bool MapEvent(signed int id, const char* sender, float w1,float w2,float w3);
  };
} // namespace mitk

#endif

