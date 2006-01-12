#include "mitkConferenceKit.h"

namespace mitk{

  ConferenceKit* ConferenceKit::m_Instance = NULL;

  ConferenceKitFactory* ConferenceKit::m_ConferenceKitFactory = NULL;

  void ConferenceKit::SetFactory( ConferenceKitFactory* factory )
  {
    m_ConferenceKitFactory = factory;
  };

  ConferenceKit*
  ConferenceKit::GetInstance()
  {
    if ( ! m_Instance )
    {
      if (m_ConferenceKitFactory)
      {
           //m_ConferenceKitFactory = ChiliConferenceKitFactory::CreateConferenceKit()
           //Weakpointer for casting to itkSmartPointer
//           itk::WeakPointer wp = itk::WeakPointer::New();
//           wp = m_ConferenceKitFactory.CreateConferenceKit();
//           m_Instance = wp;
          m_Instance = m_ConferenceKitFactory->CreateConferenceKit();
      }
    }
    return m_Instance;
  }

 //   virtual void Launch() = 0;
 //   virtual void Close() = 0;
 //   virtual void UpdateMe() = 0;
 //   virtual void SendMITK() = 0;

}//namespace
