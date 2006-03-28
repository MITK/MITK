#include "mitkConferenceKit.h"

namespace mitk{

  ConferenceKit* ConferenceKit::m_Instance = NULL;

  ConferenceKitFactory* ConferenceKit::m_ConferenceKitFactory = NULL;

  /* Set the factory instance for further use.
   */
  void ConferenceKit::SetFactory( ConferenceKitFactory* factory )
  {
    m_ConferenceKitFactory = factory;
  };

  /* Returns the ConferenceKit instance to create the specific products.
   */
  ConferenceKit*
  ConferenceKit::GetInstance()
  {
    if ( ! m_Instance )
    {
      if (m_ConferenceKitFactory)
      {
          m_Instance = m_ConferenceKitFactory->CreateConferenceKit();
      }
    }
    return m_Instance;
  }

}//namespace
