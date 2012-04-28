#ifndef MITKMATERIALEDITORPLUGINACTIVATOR_H
#define MITKMATERIALEDITORPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

  class MaterialEditorPluginActivator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
      Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

  }; // MaterialEditorPluginActivator

}

#endif // MITKMATERIALEDITORPLUGINACTIVATOR_H