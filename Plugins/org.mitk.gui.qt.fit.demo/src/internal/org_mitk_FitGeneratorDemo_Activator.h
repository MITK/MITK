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

#ifndef org_mitk_gui_qt_fit_demo_Activator_h
#define org_mitk_gui_qt_fit_demo_Activator_h

#include <ctkPluginActivator.h>

    class org_mitk_gui_qt_fit_demo_Activator : public QObject, public ctkPluginActivator
    {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mitk_gui_qt_fit_demo")
    Q_INTERFACES(ctkPluginActivator)

    public:

        /*!
         *  @brief          Registers the plug-in. Gets called by the framework when the plug-in is
         *                  first loaded.
         *  @param context  The corresponding CTK plug-in context in which the plug-in is loaded.
         */
        void start(ctkPluginContext* context);

        /*!
         *  @brief          Deregisters the plug-in. Gets called by the framework when the plug-in
         *                  is unloaded.
         *  @param context  The corresponding CTK plug-in context in which the plug-in was loaded.
         */
        void stop(ctkPluginContext* context);
    };

#endif
