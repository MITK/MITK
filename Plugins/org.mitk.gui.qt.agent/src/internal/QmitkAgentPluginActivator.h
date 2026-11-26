/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentPluginActivator_h
#define QmitkAgentPluginActivator_h

#include <ctkPluginActivator.h>

class QmitkAgentPluginActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_agent")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  /** \brief The context of the active plugin, or nullptr. */
  static ctkPluginContext* GetContext();

  /** \brief The activator of the active plugin, which relays events between its parts, or nullptr. */
  static QmitkAgentPluginActivator* GetInstance();

signals:
  /** \brief Claude Code signed in or out, maybe with another account than before. */
  void ClaudeCodeAccountChanged();

private:
  static ctkPluginContext* s_Context;
  static QmitkAgentPluginActivator* s_Instance;
};

#endif
