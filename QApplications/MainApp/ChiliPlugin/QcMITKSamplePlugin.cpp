#include <plugin.xpm>
#include <qclightbox.h>
#include <qclightboxmanager.h>
#include <QcMITKTask.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qptrlist.h>
#include <qbuttongroup.h>
#include <qapplication.h>

#include <mitkLightBoxImageReader.h>
#include <mitkDataTreeHelper.h>
#include <mitkConferenceToken.h>
#include <QmitkEventCollector.h>
#include <mitkConferenceEventMapper.h>
#include <QmitkChili3ConferenceKitFactory.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkCommonFunctionality.h>

#include "SampleApp.h"
#include "ToolBar.h"
#include "mitkChiliPlugin.h"
#include "mitkChiliPluginFactory.h"
#include "QcMITKSamplePlugin.h"

#include <ipPic.h>
#include <ipFunc.h>
#include <ipDicom/ipDicom.h>


// create event-logging object (ConferenceKit)
Chili3ConferenceKitFactory chiliconferencekitfactory;

QcPlugin* QcMITKSamplePlugin::s_PluginInstance = NULL;

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
  : QcPlugin( parent ), 
  app(NULL),
  m_Activated(false),
  m_IsFilledDataTree(true)
{

  task = new QcMITKTask( xpm(), parent, name() );

  toolbar = new ToolBar(task,this);

  QButtonGroup* tb = toolbar->GetToolBar();

  connect( toolbar, SIGNAL(LightboxSelected(QcLightbox*)), this, SLOT(selectSerie(QcLightbox*)) );
  connect( toolbar, SIGNAL(ChangeWidget(bool)), this, SLOT(CreateNewSampleApp(bool)) );

  s_PluginInstance = this;

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  //std::cout<< "QEventLogger initialisiert..."<<std::endl;

  mitk::ChiliPluginFactory::RegisterOneFactory();
  mitk::ChiliPlugin::SetPluginInstance(s_PluginInstance);
  
}

QString QcMITKSamplePlugin::name()
{
  return QString( "MITK PlugIn" );
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
  /////itkGenericOutputMacro(<<"lightbox filled");
  if(!task->isVisible()) return;

  QcLightboxManager *lbm=lightboxManager();
  QPtrList<QcLightbox> list;
  list=lbm->getLightboxes();
  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
  int id=tb->id(tb->selected());
  if (id>0)
  {
    if (!toolbar->KeepDataTreeNodes() )
    {
      if (m_Activated && (list.take(id-1))->isActive())
      {        
        selectSerie(lightbox);
      }
    }
  }
  else
    if (tb->find((list.find(lightbox))+1)->isOn())
          selectSerie(lightbox);

}

void QcMITKSamplePlugin::lightboxTiles (QcLightboxManager *lbm, int tiles)
{
  /////itkGenericOutputMacro(<<"lightbox tiles: "<<tiles);
  toolbar->ConnectButton(tiles);

}

void QcMITKSamplePlugin::selectSerie (QcLightbox* lightbox)
{
  /////itkGenericOutputMacro(<<"selectSerie");
  if(!toolbar->KeepDataTreeNodes() || !m_Activated)
    CreateNewSampleApp(true);

  if(lightbox==NULL || lightbox->getFrames()==0)
    return;

  m_Activated=true;
  m_IsFilledDataTree = true;

  mitk::Image::Pointer image = mitk::Image::New();

  mitk::LightBoxImageReader::Pointer reader=mitk::LightBoxImageReader::New();
  reader->SetLightBox(lightbox);

  image=reader->GetOutput();
  image->Update();

  if ( image.IsNotNull() )
  {
    unsigned long initTime = app->GetMultiWidget()->GetRenderWindow1()->GetRenderer()->GetMTime();

    mitk::DataTree::Pointer tree = app->GetTree();
    mitk::DataTreePreOrderIterator it(tree);
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData(image);

    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( reader->GetOutput() );
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
    node->SetProperty("LoadedFromChili", new mitk::BoolProperty( true ) );
    mitk::DataTreeNodeFactory::SetDefaultImageProperties(node);

    ipPicDescriptor *pic = reader->GetOutput()->GetPic();

    bool notAlreadyInDataTree = true;

    ipPicTSV_t* tsv = ipPicQueryTag(pic, "SOURCE HEADER");
    void* data;
    ipUInt4_t len;
    if (tsv)
    {
      //0x0018, 0x1030 : Protocol Name - ist bei Morphologie/Phase gleich
      //0x0008, 0x103e : Series Description - ist der in der unter "Beschreibung" stehende name (_bh/_bh_P)
      // aber: wahrscheinlich unsicher, da vom Benutzer eingegeben
      //eventuell brauchbar: 0x0018, 0x0024 : Sequence Name
      char * tmp;
      if (dicomFindElement((unsigned char*) tsv->value, 0x0018, 0x1030, &data, &len))
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        node->SetProperty("protocol", new mitk::StringProperty( tmp ));
        delete [] tmp;
      }

      if (dicomFindElement((unsigned char*) tsv->value, 0x0020, 0x0011, &data, &len))
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        int number = atoi(tmp);
        node->SetProperty("series number", new mitk::IntProperty( number ));
        delete [] tmp;
      }

      if (dicomFindElement((unsigned char*) tsv->value, 0x0008, 0x0070, &data, &len))
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        node->SetProperty("manufacturer", new mitk::StringProperty( tmp ));
        delete [] tmp;
      }

      //series description
      if ( dicomFindElement((unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len) )
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        node->SetProperty("name", new mitk::StringProperty( tmp ));
        delete [] tmp;
      }

      //patient name
      if ( dicomFindElement((unsigned char*) tsv->value, 0x0010, 0x0010, &data, &len) )
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        node->SetProperty("patient name", new mitk::StringProperty( tmp ));
        delete [] tmp;
      }

      if (dicomFindElement((unsigned char*) tsv->value, 0x0020, 0x000e, &data, &len))
      {
        dicomFindElement((unsigned char*) tsv->value, 0x0020, 0x000e, &data, &len);
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        node->SetProperty("series uid", new mitk::StringProperty( tmp ));

        mitk::DataTreeNode* existingNode = CommonFunctionality::GetFirstNodeByProperty(it,"series uid", new mitk::StringProperty( tmp ));

        if ( existingNode )
        {
          notAlreadyInDataTree = false;
        }

        delete [] tmp;
      }

      //Heart-Rate
      if ( dicomFindElement((unsigned char*) tsv->value, 0x0018, 0x1088, &data, &len) )
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        int heartRate = 1;
        heartRate = atoi(tmp);
        delete [] tmp;

        node->SetProperty("heartrate", new mitk::IntProperty( heartRate ));
      }
      else
        node->SetProperty("heartrate", new mitk::IntProperty( 0));


      //Unterscheidung nach Firma: 0x0008 0x0070
      if ( dicomFindElement((unsigned char*) tsv->value, 0x0008, 0x0070, &data, &len) )
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        std::string stext = tmp;
        delete [] tmp;

        std::string::size_type position = stext.find ("Philips",0);
        if(!(position== std::string::npos))
        {
          //Venc bei Philips war 1
          node->SetProperty("venc", new mitk::IntProperty( 1 ));
        }

        position = stext.find ("SIEMENS",0);
        if(!(position== std::string::npos))
        {
          //Auslesen des Venc-Wertes
          if ( dicomFindElement((unsigned char*) tsv->value, 0x0018, 0x1030, &data, &len) )
          {
            tmp = new char[len+1];
            strncpy(tmp, (char*)data, len);
            tmp[len]=0;

            stext = tmp;

            char* tmp2 = new char[4];
            tmp2[0]=tmp[6];
            tmp2[1]=tmp[7];
            tmp2[2]=tmp[8];
            tmp2[3]=0;

            int wert = atoi(tmp2);

            //Positionen 7, 8, 9
            node->SetProperty("venc", new mitk::IntProperty(wert));
            delete [] tmp;
            delete [] tmp2;
          }
        }
      }
    }
        //dicomFindElement((unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len);
        //sscanf( (char *) data, "%d", &imageNumber );
        //info.imageNumber=imageNumber;

    if (!node->GetProperty("name"))
    {
      ipPicTSV_t* seriesDescriptionTag = ipPicQueryTag(pic, "SERIES DESCRIPTION");
      if (seriesDescriptionTag)
      {
        int length = seriesDescriptionTag->n[0];
        char* seriesDescription = new char[length+1];
        strncpy(seriesDescription, (char*)(seriesDescriptionTag->value), length);
        seriesDescription[length] = 0;
        node->SetProperty("name", new mitk::StringProperty( seriesDescription ));
        delete [] seriesDescription;
      }
    }

    if (!node->GetProperty("patient name"))
    {
      ipPicTSV_t* patientNameTag = ipPicQueryTag(pic, "PATIENT NAME");
      if (patientNameTag)
      {
        int length = patientNameTag->n[0];
        char* patientName = new char[length+1];
        strncpy(patientName, (char*)(patientNameTag->value), length);
        patientName[length] = 0;
        node->SetProperty("patient name", new mitk::StringProperty( patientName ));
        delete [] patientName;
      }
    }

    if (!node->GetProperty("series source uid"))
    {
      ipPicTSV_t* seriesSourceUIDTag = ipPicQueryTag(pic, "SERIES SOURCE UID");
      if (seriesSourceUIDTag)
      {
        int length = seriesSourceUIDTag->n[0];
        char* seriesSourceUID = new char[length+1];
        strncpy(seriesSourceUID, (char*)(seriesSourceUIDTag->value), length);
        seriesSourceUID[length] = 0;
        node->SetProperty("series source uid", new mitk::StringProperty( seriesSourceUID ));
        delete [] seriesSourceUID;
      }
    }

    if (!node->GetProperty("series uid"))
    {
      ipPicTSV_t* seriesUIDTag = ipPicQueryTag(pic, "SERIES INSTANCE UID");
      if (seriesUIDTag)
      {
        int length = seriesUIDTag->n[0];
        char* seriesUID = new char[length+1];
        strncpy(seriesUID, (char*)(seriesUIDTag->value), length);
        seriesUID[length] = 0;
        node->SetProperty("series uid", new mitk::StringProperty( seriesUID ));

        //check if tree already contains node with this series instance uid
        mitk::DataTreeNode* existingNode = CommonFunctionality::GetFirstNodeByProperty(it,"series uid", new mitk::StringProperty( seriesUID ));
        if ( existingNode )
        {
          notAlreadyInDataTree = false;
        }
        delete [] seriesUID;
      }
    }

    ipPicTSV_t* imageTypeTag = ipPicQueryTag(pic, "IMAGE TYPE");
    if (imageTypeTag)
    {
      int length = imageTypeTag->n[0];
      char* imageType = new char[length+1];
      strncpy(imageType, (char*)(imageTypeTag->value), length);
      imageType[length] = 0;
      if (imageType)
      {
        std::string imageTypeString(imageType);
        if (imageTypeString == "Image, Segmentation")
          node->SetProperty("segmentation",new mitk::BoolProperty(true));
      }
      delete [] imageType;
    }

    //is source image already in data tree?
    mitk::DataTreeNode* fatherNode = NULL;
    if (node->GetProperty("series source uid"))
    {
      mitk::StringProperty::Pointer seriesSourceeUID = dynamic_cast<mitk::StringProperty*>(node->GetProperty("series source uid").GetPointer());
      fatherNode = CommonFunctionality::GetFirstNodeByProperty(it,"series uid", seriesSourceeUID);
    }

    if (notAlreadyInDataTree)
    {
      if (fatherNode)
      {
        mitk::DataTreeIteratorClone fatherIt = mitk::DataTreeHelper::FindIteratorToNode(app->GetTree(), fatherNode );
        fatherIt->Add(node);
      }
      else
        it.Add(node);

      //now that the node is added:
      //are child nodes of it already in the data tree?
      //if so the sorting of the tree has to be changed
      mitk::StringProperty::Pointer seriesInstanceUID = dynamic_cast<mitk::StringProperty*>(node->GetProperty("series uid").GetPointer());
      mitk::DataTreeIteratorClone fatherIt = mitk::DataTreeHelper::FindIteratorToNode(app->GetTree(), node );
      if (seriesInstanceUID.IsNotNull())
      {
        mitk::DataTreePreOrderIterator treeIt( app->GetTree() );
        treeIt.GoToBegin();
        bool iterate = true;
        while (!treeIt.IsAtEnd())
        {
          mitk::DataTreeNode* curNode = treeIt.Get();
          if (fatherIt->ChildPosition(curNode) == -1)
          {
            mitk::StringProperty::Pointer seriesSourceUID = dynamic_cast<mitk::StringProperty*>(curNode->GetProperty("series source uid").GetPointer());
            if (seriesSourceUID.IsNotNull())
            {
              if (*seriesSourceUID == *seriesInstanceUID)
              {
                mitk::DataTreeIteratorClone itClone = treeIt.Clone();
                itClone->GoToParent();
                int childPos = itClone->ChildPosition(curNode);
                ++treeIt;
                iterate = false;
                if (itClone->RemoveChild(childPos))
                  fatherIt->Add(curNode);
              }
            }
          }
          if (iterate)
            ++treeIt;
          else iterate = true;
        }
      }
    }

    if((app->GetMultiWidget()->GetRenderWindow1()->GetRenderer()->GetMTime()==initTime) && (app->GetStandardViewsInitialized()==false))
    {
      app->SetStandardViewsInitialized(app->GetMultiWidget()->InitializeStandardViews(&it));
    }

    app->GetMultiWidget()->RequestUpdate();
    app->GetMultiWidget()->Fit();

  }

}

void QcMITKSamplePlugin::CreateNewSampleApp(bool force)
{
  // don instantiate new app if an emptyone allready esixts
  if(m_Activated || force)
  {
    if(app)
      delete app;
    
    app = new SampleApp(task, "sample", 0);

    toolbar->SetWidget(app);
  }
}

// teleconference methods

void QcMITKSamplePlugin::connectPartner()
{
  //std::cout<< "MITKchili Connected..."<<std::endl;
  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
  tb->find(7)->setText(QString("Conference online"));

  mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
  ct->ArrangeToken();

}

void QcMITKSamplePlugin::disconnectPartner()
{
  //std::cout<< "MITKchili Disonnected..."<<std::endl;
  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
  tb->find(7)->setText(QString("Conference offline"));
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

  QButtonGroup* tb;
  tb=toolbar->GetToolBar();

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
//       tb->find(7)->setText(QString( "ID:%1 von %7 POS(%2,%3,%4) 2D(%4,%5)" )
//                    .arg(eventID).arg( w1 ).arg( w2 ).arg( w3 ).arg( p1 ).arg( p2 ).arg(str));
         tb->find(7)->setText(QString( "ID:%1 von %2").arg(eventID).arg(str));

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
