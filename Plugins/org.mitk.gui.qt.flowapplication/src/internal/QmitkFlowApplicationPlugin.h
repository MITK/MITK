/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFlowApplicationPlugin_h
#define QmitkFlowApplicationPlugin_h

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

private:

  static QmitkFlowApplicationPlugin* inst;

  ctkPluginContext* _context;
};

#endif
