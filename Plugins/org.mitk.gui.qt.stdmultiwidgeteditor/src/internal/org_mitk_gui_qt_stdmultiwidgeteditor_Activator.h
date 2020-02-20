/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef org_mitk_gui_qt_stdmultiwidgeteditor_Activator_H_
#define org_mitk_gui_qt_stdmultiwidgeteditor_Activator_H_

#include <ctkPluginActivator.h>

/**
 * \ingroup org_mitk_gui_qt_stdmultiwidgeteditor
 */
class org_mitk_gui_qt_stdmultiwidgeteditor_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_stdmultiwidgeteditor")
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

  static ctkPluginContext* GetContext();

private:

  static ctkPluginContext* m_Context;

};

typedef org_mitk_gui_qt_stdmultiwidgeteditor_Activator PluginActivator;

#endif /* org_mitk_gui_qt_stdmultiwidgeteditor_Activator_H_ */

