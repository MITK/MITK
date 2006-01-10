#include <mitkChiliConferenceKitFactory.h>
#include <qmitkQChili3Conference.h>

namespace mitk{

ConferenceKit::Pointer ChiliConferenceKitFactory::m_QCInstance = NULL;

ChiliConferenceKitFactory::ChiliConferenceKitFactory(){};
ChiliConferenceKitFactory::~ChiliConferenceKitFactory(){};

ConferenceKit::Pointer
ChiliConferenceKitFactory::GetConferenceKit()
{
  if(m_QCInstance.IsNull())
  {
    m_QCInstance = QChili3Conference::New();
  }
  return m_QCInstance;
};

}//namespace
