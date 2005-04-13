#include <chili/task.h>
#include <chili/plugin.xpm>
#include <chili/qclightbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qptrlist.h>
#include <qbuttongroup.h>
#include <mitkDataTree.h>
#include <QmitkStdMultiWidget.h>
#include <mitkLightBoxImageReader.h>
#include "SampleApp.h"
#include "QcMITKSamplePlugin.h"
#include "mitkProperties.h"
#include "mitkLevelWindowProperty.h"
#include "mitkStringProperty.h"
#include "ToolBar.h"
#include <mitkChiliPlugin.h>

#include <qmessagebox.h>

#include "ipFunc.h"


QcPlugin* QcMITKSamplePlugin::s_PluginInstance = NULL;

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
  : QcPlugin( parent )
{
  task = new QcTask( xpm(), parent, name() );

  ap = new SampleApp(task,"sample",0);
  toolbar =new ToolBar(task,this);
  toolbar->SetWidget(ap);

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
    mitk::DataTree::Pointer tree = ap->GetTree();
    mitk::DataTreePreOrderIterator it(tree);
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData(image);
    it.Add(node);
    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto( reader->GetOutput() );
    levWinProp->SetLevelWindow( levelwindow );
    node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
    node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );
    node->SetProperty("LoadedFromChili", new mitk::BoolProperty( true ) );

    ipPicTSV_t* uid= ipPicQueryTag(reader->GetOutput()->GetPic(),"IMAGE INSTANCE UID");
    ipPicTSV_t* description= ipPicQueryTag(reader->GetOutput()->GetPic(),"SERIES DESCRIPTION");
    ipPicTSV_t* patientName= ipPicQueryTag(reader->GetOutput()->GetPic(),"PATIENT NAME");

    if (description)
      node->SetProperty("name", new mitk::StringProperty( (char*)uid->value ));
    else if (uid)
      node->SetProperty("name", new mitk::StringProperty( (char*)description->value ));

    else if (patientName)
      node->SetProperty("name", new mitk::StringProperty( (char*)patientName->value ));
    else 
      node->SetProperty("name", new mitk::StringProperty( lightbox->name() ));

    ap->getMultiWidget()->Repaint();
    ap->getMultiWidget()->Fit();

  }

}


void QcMITKSamplePlugin::CreateNewSampleApp()
{
  delete ap;
  ap = new SampleApp(task,"sample",0);
  toolbar->SetWidget(ap);
}
