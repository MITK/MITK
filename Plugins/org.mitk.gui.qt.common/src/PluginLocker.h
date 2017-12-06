#pragma once

#include <QString>

#include <berryIFolderLayout.h>

#include <org_mitk_gui_qt_common_Export.h>

class MITK_QT_COMMON PluginLocker
{
public:
  PluginLocker(QString viewId);
  PluginLocker(std::string viewId);
  ~PluginLocker();

private:
  PluginLocker(PluginLocker&) = delete;
  PluginLocker& operator=(PluginLocker&) = delete;

  std::map<QString, berry::TabLock>& m_PartTabItems;
  berry::TabLock* m_TabLock;
};
