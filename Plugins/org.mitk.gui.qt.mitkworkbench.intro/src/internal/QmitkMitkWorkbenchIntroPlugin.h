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

  void start(ctkPluginContext*) override;

  QString GetQtHelpCollectionFile() const;

private:

  static QmitkMitkWorkbenchIntroPlugin* inst;

  ctkPluginContext* context;
};

#endif
