#include <chili/task.h>
#include <chili/plugin.xpm>
#include <chili/qclightbox.h>
#include <qlayout.h>
#include <qstring.h>
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
#include "ToolBar.h"

#include <qlabel.h>

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
  : QcPlugin( parent )
{
  task = new QcTask( xpm(), parent, name() );
  
  //QGridLayout* layout = new QGridLayout(task,0,0,0,-1,"plugin_area");
  
  ap = new SampleApp(task,"sample",0);
  toolbar =new ToolBar(task);
  toolbar->SetWidget(ap);

  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
  for (int i=1;i<5;++i)
    connect(tb->find(i),SIGNAL(toggled(bool)),this,SLOT(ConnectWidget(bool)));
  connect(tb->find(5),SIGNAL(toggled(bool)),this,SLOT(Reinitialize(bool)));

  idLightbox=0;
  activated =false;
  //ap = new SampleApp(task,"sample",0);
  //ap->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);  
  //layout->addWidget(ap,0,0,0);
  //layout->activate();
 
  itkGenericOutputMacro(<<"hallo");
  
  //funktioniert nicht:
  //QcLightbox* lightbox;
  //lightbox=lightboxManager()->getActiveLightbox();
  //if(lightbox==NULL)
  //{
  //  itkGenericOutputMacro(<<"lightbox is null");
  //}
  //else
  //{
  //  itkGenericOutputMacro(<<"lightbox is not null");
  //}
  //connect(lightbox,SIGNAL(lightboxActivated(QcLightbox*)),this,SLOT(selectSerie (QcLightbox*)));
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
  if (activated&&(list.take(idLightbox))->isActive())
    selectSerie(lightbox);
}

void 	QcMITKSamplePlugin::lightboxTiles (QcLightboxManager *lbm, int tiles)
{
  itkGenericOutputMacro(<<"lightbox tiles: "<<tiles);
  toolbar->ConnectButton(tiles);
  
}

void QcMITKSamplePlugin::ConnectWidget(bool on)
{
  itkGenericOutputMacro(<<"connect widget");
  QButtonGroup* tb;
  tb=toolbar->GetToolBar();
  int id=tb->id(tb->selected());
    
  if (on)
  { 
    ap = new SampleApp(task,"sample",0);
    toolbar->SetWidget(ap);
    selectLightbox(id-1);
  }
       
}

void QcMITKSamplePlugin::selectLightbox(int id)
{
  activated=true;
  idLightbox=id;

  QcLightboxManager *lbm=lightboxManager();
  QPtrList<QcLightbox> list;
  QcLightbox* lb;

  list=lbm->getLightboxes();  
  lb=list.take(id);
  selectSerie(lb);
}


void QcMITKSamplePlugin::selectSerie (QcLightbox* lightbox)
{
  itkGenericOutputMacro(<<"selectSerie");

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

  ap->getMultiWidget()->texturizePlaneSubTree(&it);
  ap->getMultiWidget()->updateMitkWidgets();
  ap->getMultiWidget()->fit();
}

void QcMITKSamplePlugin::Reinitialize(bool on)
{
  if (on)
  {
    QButtonGroup* tb;
    tb=toolbar->GetToolBar();
    tb->setButton(idLightbox+1);
  }   
}

