#include <QmitkStdMultiWidget.h>
#include <QmitkCommonFunctionality.h>
#include <SampleApp.h>
#include <QmitkEventCollector.h>
#include <QmitkChili3ConferenceKitFactory.h>

#include <chili/plugin.xpm>
#include <chili/qclightbox.h>
#include <chili/qclightboxmanager.h>
#include <QcMITKTask.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qptrlist.h>
#include <qbuttongroup.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qsplashscreen.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <3d-millenium_png_embedded.h>

#include <mitkConferenceToken.h>
#include <mitkConferenceEventMapper.h>

#include <QcMITKSamplePlugin.h>

#include <mitkChiliPlugin.h>
#include <mitkChiliPluginFactory.h>

#include <ipPic/ipPic.h>
#include <ipFunc/ipFunc.h>
#include <ipDicom/ipDicom.h>

// create event-logging object (ConferenceKit)
Chili3ConferenceKitFactory chiliconferencekitfactory;

QcPlugin* QcMITKSamplePlugin::s_QmitkPluginInstance = NULL;

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
  : QcPlugin( parent ),
  app(NULL),
  m_MITKPluginInstance(NULL)
{
  task = new QcMITKTask( xpm(), parent, name() );
  s_QmitkPluginInstance = this;

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  mitk::ChiliPluginFactory::RegisterOneFactory();
  m_MITKPluginInstance = dynamic_cast<mitk::ChiliPluginImpl*>( mitk::ChiliPlugin::GetInstance() );

  assert( m_MITKPluginInstance );

  m_MITKPluginInstance->SetPluginInstance( s_QmitkPluginInstance);
}

QString QcMITKSamplePlugin::name()
{
  return QString( "MITK" );
}

const char** QcMITKSamplePlugin::xpm()
{
  return (const char **)plugin_xpm;
}

QcMITKSamplePlugin::~QcMITKSamplePlugin()
{
  task->deleteLater();
}

extern "C" 
QcEXPORT QObject* create( QWidget *parent )
{
  return new QcMITKSamplePlugin( parent );
}

void QcMITKSamplePlugin::lightboxFilled (QcLightbox* lightbox)
{
}

void QcMITKSamplePlugin::lightboxTiles (QcLightboxManager *lbm, int tiles)
{
}

void QcMITKSamplePlugin::selectSerie (QcLightbox* lightbox)
{
}

// teleconference methods

void QcMITKSamplePlugin::connectPartner()
{
  mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
  ct->ArrangeToken();
}

void QcMITKSamplePlugin::disconnectPartner()
{
  mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
  ct->SetToken( 0 );
}

void QcMITKSamplePlugin::handleMessage( ipInt4_t type, ipMsgParaList_t *list )
{
  //QString message;
  //QString arguments;

  ipInt4_t eventID;
  ipFloat4_t w1,w2,w3,p1,p2;
  ipInt4_t sender,receiver,etype,estate,ebuttonstate,ekey;
  char *str = NULL;

  switch( type )
  {
    case mitk::m_QmitkChiliPluginConferenceID + mitk::MITKc:
      ipMsgListToVar( list,
                      ipTypeInt4, &eventID,
                      ipTypeStringPtr, &str,
                      ipTypeInt4, &etype, 
                      ipTypeInt4, &estate, 
                      ipTypeInt4, &ebuttonstate, 
                      ipTypeInt4, &ekey, 
                      ipTypeFloat4, &w1,
                      ipTypeFloat4, &w2,
                      ipTypeFloat4, &w3,
                      ipTypeFloat4, &p1,
                      ipTypeFloat4, &p2);

      if(!mitk::ConferenceEventMapper::MapEvent(eventID, str,etype,estate,ebuttonstate,ekey,w1,w2,w3,p1,p2))
        //std::cout<<"EventMaper no funciona"<<std::endl;

    break;
    case mitk::m_QmitkChiliPluginConferenceID +mitk::QTc :
      ipMsgListToVar( list,
                      ipTypeStringPtr, &str );
      //std::cout<<"CONFERENCE: (1) "<<str;
      if( ! EventCollector::PostEvent(QString(str), task))
      //std::cout<<" NO SUCCES!"<<std::endl;
      //std::cout<<std::endl;

    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::ARRANGEc:
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &sender );
        //std::cout<<"CONFERENCE TOKEN ID:  "<<sender<<std::endl;

        mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
        ct->ArrangeToken(sender);
        }
    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENREQUESTc:
        //FROM: AskForToken(...)
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &receiver );
        //std::cout<<"CONFERENCE TOKEN REQUEST FROM:  "<<receiver<<std::endl;

        mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
        ct->GetToken(receiver);
        }
    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENSETc:
        //FROM: AskForToken(...)
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &sender,
                        ipTypeInt4, &receiver );
        //std::cout<<"CONFERENCE TRY SET TOKEN FROM:  "<<sender<<" TO:"<<receiver<<std::endl;

        mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
        ct->SetToken( receiver );
        }
    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::MOUSEMOVEc:
      ipMsgListToVar( list,
                      ipTypeStringPtr, &str,
                      ipTypeFloat4, &w1,
                      ipTypeFloat4, &w2,
                      ipTypeFloat4, &w3
          );

      if( !mitk::ConferenceEventMapper::MapEvent( str, w1, w2, w3 ) )
        //std::cout<<"QcMITKSamplePlugin::handleMessage() -> MouseMoveEvent couldn't pass through"<<std::endl;

    break;   
    default:
//       message = QString( " type %1" ).arg( type );
//       arguments = "argumente unbekannt";
      break;
  }
}

// plugin methods

void QcMITKSamplePlugin::studySelected( study_t* study )
{
  static bool done = false;
  if( !done )
  {
    QVBoxLayout* layout = new QVBoxLayout( task );
    app = new SampleApp( task, "sample", 0 );
    layout->addWidget( app );

    done = true;
  }

  if (study)
  {
    //set StudyInformation
    mitk::ChiliPlugin::StudyInformation newStudy;
    newStudy.StudyInstanceUID  = study->instanceUID;
    newStudy.StudyDescription   = study->description;
    newStudy.StudyID                = study->id;
    newStudy.StudyDate             = study->date;
    newStudy.StudyTime           = study->time;
    newStudy.Modality               = study->modality;

    m_MITKPluginInstance->m_CurrentStudy = newStudy; //this is freind of ChiliPluginImpl
    m_MITKPluginInstance->m_CurrentSeries.clear();

    //get new OID for iterate
    std::string currentStudyOID = study->oid;
    char* currentStudyOID_c = (char*)currentStudyOID.c_str();
    //iterate
    iterateSeries( currentStudyOID_c, NULL, &QcMITKSamplePlugin::GlobalIterateSeriesCallback, this );
    //send event for all application listeners
    m_MITKPluginInstance->SendStudySelectedEvent();
  }
  else
  {
    std::cout<< "No Study selected." <<std::endl;
    m_MITKPluginInstance->m_CurrentSeries.clear();
    mitk::ChiliPlugin::StudyInformation emptyList;
    m_MITKPluginInstance->m_CurrentStudy = emptyList;
  }
}

ipBool_t QcMITKSamplePlugin::GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data )
{
  QcMITKSamplePlugin* callingObject = static_cast<QcMITKSamplePlugin*>(user_data);

  mitk::ChiliPlugin::SeriesInformation newSeries;
  newSeries.SeriesDescription = series->description;
  newSeries.SeriesUID             = series->instanceUID;
  newSeries.Seriesiod             = series->oid;

  callingObject->m_MITKPluginInstance->m_CurrentSeries.push_back( newSeries );

  return ipTrue; // enum from ipTypes.h
}

