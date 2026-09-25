/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMitkWorkbenchIntroPlugin_h
#define QmitkMitkWorkbenchIntroPlugin_h

#include <berryAbstractUICTKPlugin.h>

#include <QString>

namespace berry
{
  struct IQtStyleManager;
}

class QmitkMitkWorkbenchIntroPlugin : public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_mitkworkbench_intro")
  Q_INTERFACES(ctkPluginActivator)

public:

  QmitkMitkWorkbenchIntroPlugin();
  ~QmitkMitkWorkbenchIntroPlugin() override;

  static QmitkMitkWorkbenchIntroPlugin* GetDefault();

  ctkPluginContext* GetPluginContext() const;

  /**
   * \brief Returns the workbench style manager, or nullptr if it is not available.
   */
  berry::IQtStyleManager* GetStyleManager() const;

  /**
   * \brief Returns how often the plugin was started, including this time.
   *
   * The count only grows in sessions that show the welcome screen, which is
   * when this plugin is activated.
   */
  int GetStartCount() const;

  void start(ctkPluginContext*) override;

  QString GetQtHelpCollectionFile() const;

private:

  static QmitkMitkWorkbenchIntroPlugin* inst;

  ctkPluginContext* context;
  int m_StartCount;
};

#endif
