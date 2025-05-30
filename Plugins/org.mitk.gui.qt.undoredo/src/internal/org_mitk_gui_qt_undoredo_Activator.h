#ifndef org_mitk_gui_qt_undoredo_Activator_h
#define org_mitk_gui_qt_undoredo_Activator_h

#include <ctkPluginActivator.h>

class org_mitk_gui_qt_undoredo_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_undoredo")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;
};

#endif // org_mitk_gui_qt_undoredoview_Activator_h
