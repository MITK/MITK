#ifndef TEXT_LOGGER_CONFERENCE_H
#define TEXT_LOGGER_CONFERENCE_H

#include <mitkConferenceKit.h>

class TextLoggerConferenceKit:public mitk::ConferenceKit
{

  public:
    TextLoggerConferenceKit();
    ~TextLoggerConferenceKit();

    void Launch();
    void SendQt(const char* s);
    void SendMITK(signed int eventID, const char* sender, int etype, int estate, int ebuttonstate, int ekey, float w1, float w2, float w3, float p1, float p2);
    void MouseMove( const char* sender, float w1, float w2, float w3 );
    void MyTokenPriority(long int tid);
    void AskForToken(long int requester);
    void SetToken(long int sender, long int requester);

  private:
};

#endif

