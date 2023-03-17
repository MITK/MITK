/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkExtApplicationPlugin_h
#define QmitkExtApplicationPlugin_h

#include <berryAbstractUICTKPlugin.h>

#include <QString>

class QmitkExtApplicationPlugin : public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_extapplication")
  Q_INTERFACES(ctkPluginActivator)

public:

  QmitkExtApplicationPlugin();
  ~QmitkExtApplicationPlugin() override;

  static QmitkExtApplicationPlugin* GetDefault();

  ctkPluginContext* GetPluginContext() const;

  void start(ctkPluginContext*) override;

  QString GetQtHelpCollectionFile() const;

private:

  static QmitkExtApplicationPlugin* inst;

  ctkPluginContext* context;
};

#endif
