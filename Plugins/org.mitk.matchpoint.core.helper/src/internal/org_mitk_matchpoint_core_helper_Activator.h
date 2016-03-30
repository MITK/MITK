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
    ~org_mitk_matchpoint_core_helper_Activator();

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

    static org_mitk_matchpoint_core_helper_Activator* getDefault();

    static ctkPluginContext* getContext();

private:

    static ctkPluginContext* m_Context;
    static org_mitk_matchpoint_core_helper_Activator* m_Instance;

}; // org_mitk_gui_qt_algorithmcontrol_Activator

#endif // org_mitk_gui_qt_algorithmcontrol_Activator_h
