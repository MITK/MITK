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
  task = new QcTask( xpm(), parent, name() );
  QGridLayout* layout = new QGridLayout(task,0,0,0,-1,"plugin_area");
  SampleApp *ap; 
  ap = new SampleApp(task,"sample",0);

  ap->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  layout->addWidget(ap,0,0,0);
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
