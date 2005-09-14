#include <chili/task.h>
#include <chili/plugin.xpm>
#include <chili/qclightbox.h>
#include <chili/qclightboxmanager.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qptrlist.h>
#include <qbuttongroup.h>
#include <mitkDataTree.h>
#include <QmitkStdMultiWidget.h>
#include <mitkLightBoxImageReader.h>
#include "SampleApp.h"
#include "QcMITKSamplePlugin.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkDataTreeHelper.h"
#include "mitkProperties.h"
#include "mitkLevelWindowProperty.h"
#include "mitkStringProperty.h"
#include "ToolBar.h"
#include <mitkChiliPlugin.h>

#include <qmessagebox.h>

#include <ipPic.h>
#include <ipFunc.h>
#include <ipDicom/ipDicom.h>

#include "QmitkCommonFunctionality.h"



QcPlugin* QcMITKSamplePlugin::s_PluginInstance = NULL;

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
  : QcPlugin( parent ), ap(NULL)
{
  task = new QcTask( xpm(), parent, name() );

  toolbar =new ToolBar(task,this);

  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
    
  connect(toolbar,SIGNAL(LightboxSelected(QcLightbox*)),this,SLOT(selectSerie(QcLightbox*)));
  connect(toolbar,SIGNAL(ChangeWidget()),this,SLOT(CreateNewSampleApp()));
  activated =false;
   
  s_PluginInstance = this;

  mitk::ChiliPlugin::SetPluginInstance(s_PluginInstance);
}

QString 
QcMITKSamplePlugin::name() 
{ 
  return QString( "MITK PlugIn" );
}

const char ** 
QcMITKSamplePlugin::xpm()
{ 
  return (const char **)plugin_xpm; 
}

QcMITKSamplePlugin::~QcMITKSamplePlugin ()
{
  task->deleteLater();
}

extern "C" QcEXPORT QObject *
create( QWidget *parent )
{
return new QcMITKSamplePlugin( parent );  
}

void 	QcMITKSamplePlugin::lightboxFilled (QcLightbox* lightbox)
{
  /////itkGenericOutputMacro(<<"lightbox filled");
  QcLightboxManager *lbm=lightboxManager();
  QPtrList<QcLightbox> list;
  list=lbm->getLightboxes();
  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
  int id=tb->id(tb->selected());
  if (id>0)
  {
    if (!tb->find(6)->isOn())
      if (activated&&(list.take(id-1))->isActive())
          selectSerie(lightbox);
  }
  else
    if (tb->find((list.find(lightbox))+1)->isOn())
          selectSerie(lightbox);
       
}

void 	QcMITKSamplePlugin::lightboxTiles (QcLightboxManager *lbm, int tiles)
{
  /////itkGenericOutputMacro(<<"lightbox tiles: "<<tiles);
  toolbar->ConnectButton(tiles);
  
}

void QcMITKSamplePlugin::selectSerie (QcLightbox* lightbox)
{
  /////itkGenericOutputMacro(<<"selectSerie");
  activated=true;
  if(lightbox->getFrames()==0)
    return;
    
  mitk::Image::Pointer image = mitk::Image::New();
  
  mitk::LightBoxImageReader::Pointer reader=mitk::LightBoxImageReader::New();
  reader->SetLightBox(lightbox);
  
  image=reader->GetOutput();
  image->Update();

  if ( image.IsNotNull() )
  {
    unsigned long initTime = ap->GetMultiWidget()->GetRenderWindow1()->GetRenderer()->GetMTime();

    mitk::DataTree::Pointer tree = ap->GetTree();
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
        mitk::DataTreeIteratorClone fatherIt = mitk::DataTreeHelper::FindIteratorToNode(ap->GetTree(), fatherNode );
        fatherIt->Add(node);
      }
      else 
        it.Add(node);

      //now that the node is added:
      //are child nodes of it already in the data tree?
      //if so the sorting of the tree has to be changed
      mitk::StringProperty::Pointer seriesInstanceUID = dynamic_cast<mitk::StringProperty*>(node->GetProperty("series uid").GetPointer());
      mitk::DataTreeIteratorClone fatherIt = mitk::DataTreeHelper::FindIteratorToNode(ap->GetTree(), node );
      if (seriesInstanceUID != NULL)
      {
        mitk::DataTreePreOrderIterator treeIt( ap->GetTree() );
        treeIt.GoToBegin();
        bool iterate = true;
        while (!treeIt.IsAtEnd())
        {
          mitk::DataTreeNode* curNode = treeIt.Get();
          if (fatherIt->ChildPosition(curNode) == -1)
          {
            mitk::StringProperty::Pointer seriesSourceUID = dynamic_cast<mitk::StringProperty*>(curNode->GetProperty("series source uid").GetPointer());
            if (seriesSourceUID != NULL)
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
    
    if((ap->GetMultiWidget()->GetRenderWindow1()->GetRenderer()->GetMTime()==initTime) && (ap->GetStandardViewsInitialized()==false))
    {
      ap->SetStandardViewsInitialized(ap->GetMultiWidget()->InitializeStandardViews(&it));
    }

    ap->GetMultiWidget()->RequestUpdate();
    ap->GetMultiWidget()->Fit();

  }

}


void QcMITKSamplePlugin::CreateNewSampleApp()
{
  if(ap!=NULL)
    delete ap;
  ap = new SampleApp(task,"sample",0);
  toolbar->SetWidget(ap);
}
