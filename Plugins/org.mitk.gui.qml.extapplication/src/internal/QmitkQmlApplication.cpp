/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkQmlApplication.h"

#include <berryPlatformUI.h>
#include <ctkServiceReference.h>
#include <mitkIDataStorageService.h>

#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QtQml>
#include <QDebug>
#include <QQmlComponent>

#include "QmitkQmlWorkbench.h"


QmitkQmlApplication::QmitkQmlApplication()
{
}

QmitkQmlApplication::QmitkQmlApplication(const QmitkQmlApplication& other)
: QObject(other.parent())
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QVariant QmitkQmlApplication::Start(berry::IApplicationContext* /*context*/)
{
    QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());
    
    QQmlApplicationEngine engine;
    
    QmlMitkWorkbench::initialize(engine);
    engine.load(QmlMitkWorkbench::workbench);
    
    display->RunEventLoop();
    
    return EXIT_OK;
}

void QmitkQmlApplication::Stop()
{

}
