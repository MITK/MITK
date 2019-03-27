/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKPLUGINACTIVATOR_H
#define MITKPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk
{
  class MxNMultiWidgetActivator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_mxnmultiwidgeteditor")
    Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

    static ctkPluginContext* GetContext();

  private:

    static ctkPluginContext* m_Context;

  };
}
#endif // MITKPLUGINACTIVATOR_H
