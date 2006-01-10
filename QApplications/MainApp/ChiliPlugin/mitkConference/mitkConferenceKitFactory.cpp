#include <mitkConferenceKitFactory.h>
#include <mitkChiliConferenceKitFactory.h>

namespace mitk{

//static ConferenceKitFactory* ConferenceKitFactory::m_Instance = NULL;
ConferenceKitFactory* ConferenceKitFactory::m_Instance = NULL;

ConferenceKitFactory*
ConferenceKitFactory::GetInstance(int ClassKey)
{
  if (m_Instance==NULL)
  {
    //logic also for other classes if desired
    //switch(ClassKey) {
    //default:
    //  m_Instance = new ChiliConferenceKitFactory;
    //}
    m_Instance = new ChiliConferenceKitFactory;
  }
  return m_Instance;
};

}//namespace

