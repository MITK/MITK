/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYLOGPLUGIN_H_
#define BERRYLOGPLUGIN_H_

#include <ctkPluginActivator.h>
#include "berryQtPlatformLogModel.h"


namespace berry {

class QtLogPlugin : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt_log")
  Q_INTERFACES(ctkPluginActivator)

public:

  QtLogPlugin();

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  static QtLogPlugin* GetInstance();

  QtPlatformLogModel* GetLogModel() const;

  ctkPluginContext* GetContext() const;

private:
  static QtLogPlugin* instance;
  QtPlatformLogModel* m_LogModel;
  ctkPluginContext* m_Context;

};

}

#endif /*BERRYLOGPLUGIN_H_*/
