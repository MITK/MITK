/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYQTPLUGINACTIVATOR_H_
#define BERRYQTPLUGINACTIVATOR_H_

#include <berryAbstractUICTKPlugin.h>

#include <berryIQtStyleManager.h>

namespace berry {

class QtPluginActivator : public QObject, public AbstractUICTKPlugin
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  QtPluginActivator();
  ~QtPluginActivator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

private:

  IQtStyleManager::Pointer styleManager;

};

}

#endif /* BERRYQTPLUGINACTIVATOR_H_ */
