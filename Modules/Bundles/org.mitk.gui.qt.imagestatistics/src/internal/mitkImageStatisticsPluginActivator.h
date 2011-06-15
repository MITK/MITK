#ifndef MITKIMAGESTATISTICSPLUGINACTIVATOR_H
#define MITKIMAGESTATISTICSPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

  class ImageStatisticsPluginActivator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

  }; // ImageStatisticsPluginActivator

}

#endif // MITKIMAGESTATISTICSPLUGINACTIVATOR_H