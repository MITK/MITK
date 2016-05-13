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

#ifndef QmlMitkDatamanger_h
#define QmlMitkDatamanger_h

#include "MitkQmlItemsExports.h"

#include <mitkDataStorage.h>
#include <QmitkDataStorageListModel.h>

#include <QQuickItem>

class MITKQMLITEMS_EXPORT QmlMitkDatamanager : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int index READ getIndex WRITE setIndex NOTIFY indexChanged);

private:
    int m_index;
    static QmitkDataStorageListModel* model;
    static mitk::DataStorage::Pointer storage;

public:
    static QmlMitkDatamanager* instance;

    int getIndex();
    void setIndex(int index);

    QmlMitkDatamanager();
    ~QmlMitkDatamanager();

    static void create(QQmlEngine &engine, mitk::DataStorage::Pointer storage);

public slots:
    void toggleVisibility(bool checked);
    void reinitNode();
    void globalReinit();
    void deleteNode();

signals:
    void indexChanged();
};

#endif
