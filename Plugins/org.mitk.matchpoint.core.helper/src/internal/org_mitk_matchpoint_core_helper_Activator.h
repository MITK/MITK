/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef org_mitk_matchpoint_core_helper_Activator_h
#define org_mitk_matchpoint_core_helper_Activator_h

#include <berryAbstractUICTKPlugin.h>

class org_mitk_matchpoint_core_helper_Activator :
    public berry::AbstractUICTKPlugin
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID "org_mitk_matchpoint_core_helper")
        Q_INTERFACES(ctkPluginActivator)

public:

    org_mitk_matchpoint_core_helper_Activator();
    ~org_mitk_matchpoint_core_helper_Activator() override;

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

    static org_mitk_matchpoint_core_helper_Activator* getDefault();

    static ctkPluginContext* getContext();

private:

    static ctkPluginContext* m_Context;
    static org_mitk_matchpoint_core_helper_Activator* m_Instance;

}; // org_mitk_gui_qt_algorithmcontrol_Activator

#endif // org_mitk_gui_qt_algorithmcontrol_Activator_h
