#ifndef MITKMOVIEMAKERPLUGINACTIVATOR_H
#define MITKMOVIEMAKERPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

  class MovieMakerPluginActivator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
      Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

  }; // MovieMakerPluginActivator

}

#endif // MITKMOVIEMAKERPLUGINACTIVATOR_H