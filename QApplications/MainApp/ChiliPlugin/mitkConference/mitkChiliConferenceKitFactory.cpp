#include <mitkChiliConferenceKitFactory.h>
#include <QmitkQChili3Conference.h>
#include <itkSmartPointer.h>

namespace mitk{
//class ConferenceKit;

//ConferenceKit::Pointer ChiliConferenceKitFactory::m_QCInstance;

ChiliConferenceKitFactory::ChiliConferenceKitFactory()
{
  ConferenceKit::SetFactory( this );
};

ChiliConferenceKitFactory::~ChiliConferenceKitFactory(){};

ConferenceKit*
ChiliConferenceKitFactory::CreateConferenceKit()
{
//   QChili3Conference::Pointer qcc = QChili3Conference::New();
//   return qcc->GetPointer();
     return new QChili3Conference;
};

}//namespace
