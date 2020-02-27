/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKFLOWAPPLICATIONLICATIONPLUGIN_H_
#define QMITKFLOWAPPLICATIONLICATIONPLUGIN_H_

#include <berryAbstractUICTKPlugin.h>

#include <QString>

class QmitkFlowApplicationPlugin : public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_flowapplication")
  Q_INTERFACES(ctkPluginActivator)

public:

  QmitkFlowApplicationPlugin();
  ~QmitkFlowApplicationPlugin() override;

  static QmitkFlowApplicationPlugin* GetDefault();

  ctkPluginContext* GetPluginContext() const;

  void start(ctkPluginContext*) override;
  void stop(ctkPluginContext* context) override;

  QString GetQtHelpCollectionFile() const;

private:

  void loadDataFromDisk(const QStringList& args, bool globalReinit);
  void startNewInstance(const QStringList& args, const QStringList &files);

private Q_SLOTS:

  void handleIPCMessage(const QByteArray &msg);

private:

  static QmitkFlowApplicationPlugin* inst;

  ctkPluginContext* _context;
};

#endif /* QMITKFLOWAPPLICATIONLICATIONPLUGIN_H_ */
