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
   
  itkGenericOutputMacro(<<"hallo");
  
  s_PluginInstance = this;
}

QString 
QcMITKSamplePlugin::name() 
{ 
  return QString( "SampleApp" ); 
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
  itkGenericOutputMacro(<<"lightbox filled");
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
  itkGenericOutputMacro(<<"lightbox tiles: "<<tiles);
  toolbar->ConnectButton(tiles);
  
}

void QcMITKSamplePlugin::selectSerie (QcLightbox* lightbox)
{
  itkGenericOutputMacro(<<"selectSerie");
  activated=true;
  if(lightbox->getFrames()==0)
    return;
 
  mitk::Image::Pointer image = mitk::Image::New();
  
  mitk::LightBoxImageReader::Pointer reader=mitk::LightBoxImageReader::New();
  reader->SetLightBox(lightbox);
  
  image=reader->GetOutput();
  image->Update();
  mitk::DataTree::Pointer tree = ap->GetTree();
  
  mitk::DataTreePreOrderIterator it(tree);

  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();

  node->SetData(image);

  it.Add(node);

  mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( reader->GetOutput()->GetPic() );
  levWinProp->SetLevelWindow( levelwindow );
  node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
  node->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );
  node->SetProperty("LoadedFromChili", new mitk::BoolProperty( true ) );
  node->SetProperty("name", new mitk::StringProperty( "1" ) );

itkGenericOutputMacro(<<"before thresh: ");
  ipPicDescriptor *cur, *tmp;
  cur = ipFuncThresh( image->GetPic(), 100, NULL );
itkGenericOutputMacro(<<"before convert: ");
  cur = ipFuncConvert( cur, ipPicUInt, 8 );
  if(cur!=NULL)
  {
    itkGenericOutputMacro(<<"before multc: ");
      tmp = ipFuncMultC( cur, 100, ipFuncKeep, NULL );
    itkGenericOutputMacro(<<"before free: ");
      ipPicFree( cur );
  ipPicPut("c:\\atest2.pic",cur); 
      cur=tmp;
    itkGenericOutputMacro(<<"after thresh: ");
      mitk::Image::Pointer result = mitk::Image::New();
      if(cur->dim>3) cur->dim=3;
      result->Initialize(cur);
      result->SetGeometry(static_cast<mitk::Geometry3D*>(image->GetGeometry()->Clone().GetPointer()));
      result->SetPicVolume(cur);
      int pos=it.ChildPosition(node);
      if(pos<0) pos=0;
      it.GoToChild( pos );
    itkGenericOutputMacro(<<"after gotoch: "<<pos);
      mitk::DataTreeNode::Pointer resultnode = mitk::DataTreeNode::New();
      resultnode->SetData(result);
    itkGenericOutputMacro(<<"before lv: ");
      levelwindow.SetAuto( cur );
      levWinProp->SetLevelWindow( levelwindow );
      resultnode->GetPropertyList()->SetProperty( "levelwindow", levWinProp );
itkGenericOutputMacro(<<"lv: "<<levelwindow.GetMin() <<" "<<levelwindow.GetMax());
    itkGenericOutputMacro(<<"before vr: ");
      resultnode->SetProperty( "volumerendering", new mitk::BoolProperty( false ) );
    itkGenericOutputMacro(<<"before addr: ");
      resultnode->SetProperty("name", new mitk::StringProperty( "2" ) );
      it.Add(resultnode);
    itkGenericOutputMacro(<<"done: ");
  }

  ap->getMultiWidget()->texturizePlaneSubTree(&it);
  ap->getMultiWidget()->updateMitkWidgets();
  ap->getMultiWidget()->fit();
}


void QcMITKSamplePlugin::CreateNewSampleApp()
{
  ap = new SampleApp(task,"sample",0);
  toolbar->SetWidget(ap);
}
