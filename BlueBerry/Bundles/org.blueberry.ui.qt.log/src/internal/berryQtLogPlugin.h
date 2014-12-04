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

#ifndef BERRYLOGPLUGIN_H_
#define BERRYLOGPLUGIN_H_

#include <ctkPluginActivator.h>

#include "berryQtPlatformLogModel.h"


namespace berry {

class QtLogPlugin : public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt_log")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  QtLogPlugin();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static QtLogPlugin* GetInstance();

  QtPlatformLogModel* GetLogModel();

private:
  static QtLogPlugin* instance;
  QtPlatformLogModel* m_LogModel;

};

}

#endif /*BERRYLOGPLUGIN_H_*/
