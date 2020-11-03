/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMATERIALEDITORPLUGINACTIVATOR_H
#define MITKMATERIALEDITORPLUGINACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

  class MaterialEditorPluginActivator :
    public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_materialeditor")
    Q_INTERFACES(ctkPluginActivator)

  public:

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

  }; // MaterialEditorPluginActivator

}

#endif // MITKMATERIALEDITORPLUGINACTIVATOR_H
