#ifndef MITKPLUGINACTIVATOR_H
#define MITKPLUGINACTIVATOR_H

// Parent classes
#include <QObject>
#include <ctkPluginActivator.h>
#include <mitkExportMacros.h>

namespace mitk
{
  class MITK_LOCAL PluginActivator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_INTERFACES(ctkPluginActivator)

  public:
    void start(ctkPluginContext *context);
    void stop(ctkPluginContext *context);
  };
}

#endif
