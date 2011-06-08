#ifndef mitkColourImageProcessingActivator_h
#define mitkColourImageProcessingActivator_h

#include <ctkPluginActivator.h>

namespace mitk {

class mitkColourImageProcessingActivator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // mitkColourImageProcessingActivator

}

#endif // mitkColourImageProcessingActivator_h