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


#ifndef BERRYEXPRESSIONPLUGIN_H
#define BERRYEXPRESSIONPLUGIN_H

#include "berryPlugin.h"

namespace berry {

class ExpressionPlugin : public Plugin
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_blueberry_core_expressions")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  static QObject* m_PluginListener;

  ExpressionPlugin();

  static ExpressionPlugin* GetDefault();

  static QString GetPluginId();

  void start(ctkPluginContext *context);
  void stop(ctkPluginContext* context);
  ctkPluginContext* GetPluginContext() const;

private:

  static ExpressionPlugin* m_Default;
};


}

#endif // BERRYEXPRESSIONPLUGIN_H
