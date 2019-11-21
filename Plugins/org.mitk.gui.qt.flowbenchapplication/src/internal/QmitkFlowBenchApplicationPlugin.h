/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef QMITKFLOWBENCHAPPLICATIONLICATIONPLUGIN_H_
#define QMITKFLOWBENCHAPPLICATIONLICATIONPLUGIN_H_

#include <berryAbstractUICTKPlugin.h>

#include <QString>

class QmitkFlowBenchApplicationPlugin : public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_flowbenchapplication")
  Q_INTERFACES(ctkPluginActivator)

public:

  QmitkFlowBenchApplicationPlugin();
  ~QmitkFlowBenchApplicationPlugin() override;

  static QmitkFlowBenchApplicationPlugin* GetDefault();

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

  static QmitkFlowBenchApplicationPlugin* inst;

  ctkPluginContext* _context;
};

#endif /* QMITKFLOWBENCHAPPLICATIONLICATIONPLUGIN_H_ */
