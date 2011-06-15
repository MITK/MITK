#ifndef MITKVOLUMETRYPLUGINACTIVATOR_H
#define MITKVOLUMETRYPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

  class VolumetryPluginActivator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
      Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

  }; // VolumetryPluginActivator

}

#endif // MITKVOLUMETRYPLUGINACTIVATOR_H