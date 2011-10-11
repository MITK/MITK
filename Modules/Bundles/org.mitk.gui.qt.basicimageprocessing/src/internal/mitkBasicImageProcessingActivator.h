#ifndef MITKBASICIMAGEPROCESSINGACTIVATOR_H
#define MITKBASICIMAGEPROCESSINGACTIVATOR_H

#include <ctkPluginActivator.h>
#include <mitkExportMacros.h>

namespace mitk {

class MITK_LOCAL BasicImageProcessingActivator :
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