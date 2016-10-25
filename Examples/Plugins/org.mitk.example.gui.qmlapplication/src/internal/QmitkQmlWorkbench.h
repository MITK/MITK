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

#ifndef QmlMitkWorkbench_h
#define QmlMitkWorkbench_h

#include <mitkDataStorage.h>

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QString>
#include <QStringList>

class QmlMitkWorkbench : public QQuickItem
{
  Q_OBJECT

public:
  static QmlMitkWorkbench *instance;
  static mitk::DataStorage::Pointer storage;
  static QUrl workbench;

  QmlMitkWorkbench();
  ~QmlMitkWorkbench();

  static void initialize(QQmlEngine &engine);

public slots:
  void loadFiles();
};

#endif
