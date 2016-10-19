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

#include "mitkQmlAppPluginActivator.h"
#include "QmitkQmlApplication.h"

#include <berryLog.h>
#include <berryMacros.h>

#include <mitkVersion.h>

#include <QFileInfo>
#include <QDateTime>

namespace mitk {

    QmlAppPluginActivator* QmlAppPluginActivator::inst = 0;

    QmlAppPluginActivator::QmlAppPluginActivator()
    {
        inst = this;
    }

    QmlAppPluginActivator::~QmlAppPluginActivator()
    {
    }

    QmlAppPluginActivator* QmlAppPluginActivator::GetDefault()
    {
        return inst;
    }

    void QmlAppPluginActivator::start(ctkPluginContext* context)
    {
        berry::AbstractUICTKPlugin::start(context);

        this->context = context;

        BERRY_REGISTER_EXTENSION_CLASS(QmitkQmlApplication, context);
    }

    ctkPluginContext* QmlAppPluginActivator::GetPluginContext() const
    {
        return context;
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtPlugin>
Q_EXPORT_PLUGIN2(my_awesomeproject_exampleapp, mitk::QmlAppPluginActivator)
#endif
