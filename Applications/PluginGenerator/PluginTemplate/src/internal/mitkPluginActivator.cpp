$(license)

#include "$(activator-file-name).h"
#include "$(view-file-name).h"

namespace mitk
{
  void $(activator-class-name)::start(ctkPluginContext *context)
  {
    BERRY_REGISTER_EXTENSION_CLASS($(view-class-name), context)
  }

  void $(activator-class-name)::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
