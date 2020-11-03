/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYOBJECTINSPECTORACTIVATOR_H
#define BERRYOBJECTINSPECTORACTIVATOR_H

#include <ctkPluginActivator.h>

namespace berry {

class org_blueberry_ui_qt_objectinspector_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt_objectinspector")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

};

typedef org_blueberry_ui_qt_objectinspector_Activator PluginActivator;

}

#endif // BERRYOBJECTINSPECTORACTIVATOR_H
