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


#ifndef QMITKEXTAPPLICATIONPLUGIN_H_
#define QMITKEXTAPPLICATIONPLUGIN_H_

#include <berryAbstractUICTKPlugin.h>

#include <QString>

#include <berryQCHPluginListener.h>

class QmitkExtApplicationPlugin : public QObject, public berry::AbstractUICTKPlugin
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_extapplication")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  QmitkExtApplicationPlugin();
  ~QmitkExtApplicationPlugin();

  static QmitkExtApplicationPlugin* GetDefault();

  ctkPluginContext* GetPluginContext() const;

  void start(ctkPluginContext*);

  QString GetQtHelpCollectionFile() const;

private:

  static QmitkExtApplicationPlugin* inst;

  ctkPluginContext* context;
};

#endif /* QMITKEXTAPPLICATIONPLUGIN_H_ */
