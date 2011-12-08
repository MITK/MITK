$(license)

#ifndef $(activator-file-name)_h
#define $(activator-file-name)_h

#include <ctkPluginActivator.h>

namespace mitk {

class $(activator-class-name) :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

}; // $(activator-class-name)

}

#endif // $(activator-file-name)_h
