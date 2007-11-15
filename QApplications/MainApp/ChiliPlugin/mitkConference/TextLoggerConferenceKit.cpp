#include <TextLoggerConferenceKit.h>
#include <fstream>

//using namespace std;

//const char* FILEPATH = "/home/hasselberg/mitkTextLogger.out";
const char* FILEPATH = "mitkTextLogger.out";

TextLoggerConferenceKit::TextLoggerConferenceKit()
{
  //ofstream MITKFile( FILEPATH, ios::trunc);
};

TextLoggerConferenceKit::~TextLoggerConferenceKit(){};

  void
TextLoggerConferenceKit::SendQt(const char* s)
{

  std::ofstream MITKGlobalFile( FILEPATH, std::ios::app);
  if ( MITKGlobalFile )
  {
    MITKGlobalFile << s << std::endl;

  }
}; 

 void
TextLoggerConferenceKit::SendMITK(signed int eventID, const char* sender, int etype, int estate, int ebuttonstate, int ekey, float w1, float w2, float w3, float p1, float p2)
{  

  std::ofstream MITKGlobalFile( FILEPATH, std::ios::app);
  if (MITKGlobalFile )
  {
    MITKGlobalFile<<eventID<<"&&"<<sender<<"&&"<<etype<<"&&"<<estate<<"&&"<< ebuttonstate<<"&&"<< ekey<<"&&"<< w1<<"&&"<< w2<<"&&"<< w3<<"&&"<<p1<<"&&"<< p2<< std::endl;
  }
};


  void
TextLoggerConferenceKit::MouseMove( const char* sender, float w1, float w2, float w3 )
{  

  std::ofstream MITKGlobalFile( FILEPATH, std::ios::app);
  if (MITKGlobalFile )
  {
    MITKGlobalFile<<sender<<"&&"<< w1<<"&&"<< w2<<"&&"<< w3<< std::endl;
  }
};


  void
TextLoggerConferenceKit::MyTokenPriority(long int /*tid*/)
{
  //  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::ARRANGEc, list );
}

  void
TextLoggerConferenceKit::AskForToken(long int /*tid*/)
{
  //  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENREQUESTc , list );
};

  void
TextLoggerConferenceKit::SetToken(long int /*sender*/, long int /*requester*/)
{
  //  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENSETc , list );
};
