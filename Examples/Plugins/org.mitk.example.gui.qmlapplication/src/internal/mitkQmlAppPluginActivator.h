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

#ifndef MITK_EXAMPLEAPP_PLUGIN_ACTIVATOR_H_
#define MITK_EXAMPLEAPP_PLUGIN_ACTIVATOR_H_

#include <QString>
#include <berryAbstractUICTKPlugin.h>

namespace mitk
{
  class QmlAppPluginActivator : public berry::AbstractUICTKPlugin
  {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org_mitk_example_gui_qmlapplication")
#endif
    Q_INTERFACES(ctkPluginActivator)

  public:
    QmlAppPluginActivator();
    ~QmlAppPluginActivator();

    static QmlAppPluginActivator *GetDefault();

    ctkPluginContext *GetPluginContext() const;

    void start(ctkPluginContext *);

  private:
    static QmlAppPluginActivator *inst;

    ctkPluginContext *context;
  };
}

#endif /* MITK_EXAMPLEAPP_PLUGIN_ACTIVATOR_H_ */
