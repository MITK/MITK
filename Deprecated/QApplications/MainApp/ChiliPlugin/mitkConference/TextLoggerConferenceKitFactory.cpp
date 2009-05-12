#include <TextLoggerConferenceKitFactory.h>
#include <TextLoggerConferenceKit.h>


TextLoggerConferenceKitFactory::TextLoggerConferenceKitFactory()
{
  mitk::ConferenceKit::SetFactory( this );
};

TextLoggerConferenceKitFactory::~TextLoggerConferenceKitFactory(){};

mitk::ConferenceKit*
TextLoggerConferenceKitFactory::CreateConferenceKit()
{
     return new TextLoggerConferenceKit;
}

