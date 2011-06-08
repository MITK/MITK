#ifndef MITKBASICIMAGEPROCESSINGACTIVATOR_H
#define MITKBASICIMAGEPROCESSINGACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

class BasicImageProcessingActivator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // basicImageProcessingActivator

}

#endif // MITKBASICIMAGEPROCESSINGACTIVATOR_H