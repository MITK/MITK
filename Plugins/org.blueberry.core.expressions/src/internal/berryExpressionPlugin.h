/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEXPRESSIONPLUGIN_H
#define BERRYEXPRESSIONPLUGIN_H

#include "berryPlugin.h"

namespace berry {

class ExpressionPlugin : public Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_blueberry_core_expressions")
  Q_INTERFACES(ctkPluginActivator)

public:

  static QObject* m_PluginListener;

  ExpressionPlugin();

  static ExpressionPlugin* GetDefault();

  static QString GetPluginId();

  void start(ctkPluginContext *context) override;
  void stop(ctkPluginContext* context) override;
  ctkPluginContext* GetPluginContext() const;

private:

  static ExpressionPlugin* m_Default;
};


}

#endif // BERRYEXPRESSIONPLUGIN_H
