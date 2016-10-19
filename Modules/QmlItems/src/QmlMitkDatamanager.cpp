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

#include "QmlMitkDatamanager.h"

#include <mitkImage.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <QQmlEngine>
#include <QQmlContext>

#include <QmlMitkProperties.h>
#include <QmlMitkTransferFunctionItem.h>

QmlMitkDatamanager* QmlMitkDatamanager::instance = nullptr;
QmitkDataStorageListModel* QmlMitkDatamanager::model = nullptr;
mitk::DataStorage::Pointer QmlMitkDatamanager::storage = nullptr;

QmlMitkDatamanager::QmlMitkDatamanager()
{
    instance = this;
}

QmlMitkDatamanager::~QmlMitkDatamanager()
{
}

void QmlMitkDatamanager::toggleVisibility(bool checked)
{
    QModelIndex modelIndex = QmlMitkDatamanager::model->index(this->m_index);
    mitk::DataNode::Pointer node = QmlMitkDatamanager::model->getNode(modelIndex);
    node->SetVisibility(checked);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmlMitkDatamanager::setIndex(int index)
{
    this->m_index = index;

    if(index >= 0)
    {
        QModelIndex modelIndex = QmlMitkDatamanager::model->index(this->m_index);
        mitk::DataNode::Pointer node = QmlMitkDatamanager::model->getNode(modelIndex);

        if(dynamic_cast<mitk::Image*>(node->GetData()) && dynamic_cast<mitk::Image*>(node->GetData())->GetDimension()>=3 )
        {
            QmlMitkProperties::instance->notify(node);
            QmlMitkTransferFunctionItem::instance->SetDataNode(node);
        }
        else
        {
            QmlMitkProperties::instance->setEnabled(false);
            emit QmlMitkProperties::instance->sync();
        }
    }
    else
    {
        QmlMitkProperties::instance->setEnabled(false);
        emit QmlMitkProperties::instance->sync();
    }

    emit this->indexChanged();
}

int QmlMitkDatamanager::getIndex()
{
    return this->m_index;
}

void QmlMitkDatamanager::reinitNode()
{
    QModelIndex modelIndex = QmlMitkDatamanager::model->index(this->m_index);
    mitk::DataNode::Pointer node = QmlMitkDatamanager::model->getNode(modelIndex);

    mitk::BaseData::Pointer basedata = node->GetData();
    if ( basedata.IsNotNull() &&
        basedata->GetTimeGeometry()->IsValid() )
    {
        mitk::RenderingManager::GetInstance()->InitializeViews(basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    }
}

void QmlMitkDatamanager::globalReinit()
{
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(QmlMitkDatamanager::storage);
}

void QmlMitkDatamanager::deleteNode()
{
    QModelIndex modelIndex = QmlMitkDatamanager::model->index(this->m_index);
    mitk::DataNode::Pointer node = QmlMitkDatamanager::model->getNode(modelIndex);

    QmlMitkDatamanager::storage->Remove(node);
    this->globalReinit();
}

void QmlMitkDatamanager::create(QQmlEngine &engine, mitk::DataStorage::Pointer storage)
{
    qmlRegisterType<QmlMitkDatamanager>("Mitk.Views", 1, 0, "DataManager");

    mitk::NodePredicateBase::Pointer filter = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true))).GetPointer();

    QmlMitkDatamanager::storage = storage;
    QmlMitkDatamanager::model = new QmitkDataStorageListModel(storage, filter);

    QQmlContext* context = engine.rootContext();
    context->setContextProperty("dataStorage", QmlMitkDatamanager::model);

    QQmlComponent component(&engine, QUrl("qrc:/views/MitkDataManager.qml"));
}
