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

#include "QmitkQmlApplication.h"

#include <berryPlatformUI.h>
#include <ctkServiceReference.h>
#include <mitkIDataStorageService.h>

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QtQml>

#include "QmitkQmlWorkbench.h"

QmitkQmlApplication::QmitkQmlApplication()
{
}

QmitkQmlApplication::QmitkQmlApplication(const QmitkQmlApplication &other) : QObject(other.parent())
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QVariant QmitkQmlApplication::Start(berry::IApplicationContext * /*context*/)
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
