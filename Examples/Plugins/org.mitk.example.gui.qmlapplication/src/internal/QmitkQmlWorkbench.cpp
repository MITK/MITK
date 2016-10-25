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

#include "QmitkQmlWorkbench.h"

#include <mitkCoreObjectFactory.h>
#include <mitkDataNode.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkStandaloneDataStorage.h>

#include <QApplication>
#include <QFileDialog>
#include <QQmlEngine>
#include <QtQml>

#include <QmlMitkDatamanager.h>
#include <QmlMitkImageNavigator.h>
#include <QmlMitkProperties.h>
#include <QmlMitkSliderLevelWindowItem.h>
#include <QmlMitkStdMultiItem.h>
#include <QmlMitkTransferFunctionItem.h>

#include <QmitkIOUtil.h>

QmlMitkWorkbench *QmlMitkWorkbench::instance = nullptr;
mitk::DataStorage::Pointer QmlMitkWorkbench::storage = nullptr;
QUrl QmlMitkWorkbench::workbench = QUrl(QStringLiteral("qrc:/MitkQuickRender.qml"));

QmlMitkWorkbench::QmlMitkWorkbench()
{
  instance = this;
}

QmlMitkWorkbench::~QmlMitkWorkbench()
{
}

void QmlMitkWorkbench::loadFiles()
{
  QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Load data", 0, QmitkIOUtil::GetFileOpenFilterString());

  try
  {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QmitkIOUtil::Load(fileNames, *QmlMitkWorkbench::storage);
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(QmlMitkWorkbench::storage);
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_ERROR << "Exception during file open: " << ex;
  }

  QApplication::restoreOverrideCursor();
}

void QmlMitkWorkbench::initialize(QQmlEngine &engine)
{
  QmlMitkWorkbench::storage = mitk::StandaloneDataStorage::New().GetPointer();

  QmlMitkSliderLevelWindowItem::create(engine, storage);
  QmlMitkStdMultiItem::create(engine, storage);
  QmlMitkDatamanager::create(engine, storage);

  QmlMitkTransferFunctionItem::create();
  QmlMitkProperties::create(engine);
  QmlMitkImageNavigator::create(engine);

  qmlRegisterType<QmlMitkWorkbench>("Mitk.Views", 1, 0, "Workbench");
}
