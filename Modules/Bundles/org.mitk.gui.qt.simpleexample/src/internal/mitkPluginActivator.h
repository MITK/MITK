#ifndef MITKPLUGINACTIVATOR_H
#define MITKPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

class PluginActivator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // PluginActivator

}

#endif // MITKPLUGINACTIVATOR_H
