#include <QmitkChili3ConferenceKitFactory.h>
#include <QmitkChili3ConferenceKit.h>


Chili3ConferenceKitFactory::Chili3ConferenceKitFactory()
{
  mitk::ConferenceKit::SetFactory( this );
}

Chili3ConferenceKitFactory::~Chili3ConferenceKitFactory(){};

mitk::ConferenceKit*
Chili3ConferenceKitFactory::CreateConferenceKit()
{
     return new Chili3Conference;
}

