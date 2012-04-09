#ifndef MITKIMAGENAVIGATORPLUGINACTIVATOR_H
#define MITKIMAGENAVIGATORPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

  class ImageNavigatorPluginActivator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
      Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

  }; // ImageNavigatorPluginActivator

}

#endif // MITKIMAGENAVIGATORPLUGINACTIVATOR_H