#include <chili/task.h>
#include <chili/plugin.xpm>
#include <chili/qclightbox.h>
#include <qlayout.h>
#include <qstring.h>
#include <mitkDataTree.h>
#include <QmitkStdMultiWidget.h>
#include <mitkLightBoxImageReader.h>
#include "SampleApp.h"
#include "QcMITKSamplePlugin.h"
#include "mitkLevelWindowProperty.h"

#include <qlabel.h>

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
  : QcPlugin( parent )
{
  task = new QcTask( xpm(), parent, name() );
  QGridLayout* layout = new QGridLayout(task,0,0,0,-1,"plugin_area");
   
  ap = new SampleApp(task,"sample",0);

  ap->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  layout->addWidget(ap,0,0,0);

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
  //connect(lightbox,SIGNAL(lightbox->lightboxActivated(QcLightbox*)),this,SLOT(selectSerie (QcLightbox*)));
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
  selectSerie(lightbox);
}

void QcMITKSamplePlugin::selectSerie (QcLightbox* lightbox)
{
  itkGenericOutputMacro(<<"selectSerie");

  
  if(lightbox->getFrames()==0)
    return;
  
  mitk::Image::Pointer image = mitk::Image::New();
  
  //Variante 1
  //ipPicDescriptor* pic;
  //pic=lightbox->fetchVolume();

  //image->Initialize(pic);
  //image->SetPicVolume(pic);

  //Variante 2
  mitk::LightBoxImageReader::Pointer reader=mitk::LightBoxImageReader::New();
  reader->SetLightBox(lightbox);
  
  image=reader->GetOutput();
  image->Update();
  mitk::DataTree::Pointer tree = ap->GetTree();
  
  mitk::DataTreeIterator* it = tree->inorderIterator();

  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();

  node->SetData(image);

  it->add(node);

  mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( reader->GetOutput()->GetPic() );
  levWinProp->SetLevelWindow( levelwindow );
  node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );


  ap->getMultiWidget()->texturizePlaneSubTree( tree->inorderIterator());
  ap->getMultiWidget()->updateMitkWidgets();
  ap->getMultiWidget()->fit();
  

}

/*
  
  ipPicDescriptor* pic;
  
  QcLightbox* lightbox;
  
  mitk::LightBoxImageReader::Pointer reader=mitk::LightBoxImageReader::New();
  reader->SetLightBox(


  connect(lightbox,lightboxReloadPics(lightbox),this,SLOT(
  
  pic= von der qclightbox
  
  mitk::Image::Pointer image = mitk::Image::New();
  
  image->Initialize(pic);
  image->SetPicVolume(pic);

  mitk::DataTree::Pointer tree = ap->GetTree();
  
  mitk::DataTreeIterator* it = tree->inorderIterator();

  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();

  node->SetData(image);

  it->add(node);



  ap->getMultiWidget()->texturizePlaneSubTree( tree->inorderIterator());
  ap->getMultiWidget()->updateMitkWidgets();
  ap->getMultiWidget()->fit();


*/