#include <chili/task.h>
#include <chili/plugin.xpm>
#include <qlayout.h>
#include <qstring.h>
#include "SampleApp.h"
#include "QcMITKSamplePlugin.h"

#include <qlabel.h>

QcMITKSamplePlugin::QcMITKSamplePlugin( QWidget *parent )
        : QcPlugin( parent )
{
  QHBoxLayout *layout;
  SampleApp *ap; 

  task = new QcTask( xpm(), parent, name() );
  ap=new SampleApp( task );
  //ap->setText( "Hello, World!" );

  layout = new QHBoxLayout( task ); 
  layout->addWidget( ap ); 
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
