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

#include "QmitkDataManagerLightView.h"
#include "mitkNodePredicateDataType.h"
#include <QtGui>

const std::string QmitkDataManagerLightView::VIEW_ID = "org.mitk.views.datamanagerlight";

struct QmitkDataManagerLightViewData
{
    // static
    mitk::NodePredicateBase::Pointer m_Predicate;

    // data
    QList<mitk::DataNode*> m_DataNodes;
    mitk::DataNode* m_SelectedNode;

    // widget
    QListWidget* m_ListWidget;

};

QmitkDataManagerLightView::QmitkDataManagerLightView()
    : d( new QmitkDataManagerLightViewData )
{
    d->m_Predicate = mitk::NodePredicateDataType::New("Image");
    d->m_SelectedNode = 0;
    d->m_ListWidget = 0;
}

QmitkDataManagerLightView::~QmitkDataManagerLightView()
{
    delete d;
}

void QmitkDataManagerLightView::NodeAdded(const mitk::DataNode *node)
{
    if( d->m_Predicate->CheckNode(node) )
    {
        mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
        d->m_DataNodes.append(nonConstNode);
        d->m_ListWidget->addItem( new QListWidgetItem( QString::fromStdString( node->GetName() ) ) );
    }
}

void QmitkDataManagerLightView::NodeRemoved(const mitk::DataNode *node)
{
    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    int index = d->m_DataNodes.indexOf(nonConstNode);
    if( index >= 0 )
    {
        d->m_DataNodes.removeAt(index);
        QListWidgetItem* item = d->m_ListWidget->takeItem(index);
        delete item;
    }
}

void QmitkDataManagerLightView::CreateQtPartControl(QWidget* parent)
{

    QPushButton* loadButton = new QPushButton("Load image");

    d->m_ListWidget = new QListWidget;

    QLabel* imageInfoLabel = new QLabel("Unknown name");

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( loadButton, 0,0 );
    layout->addWidget( d->m_ListWidget, 1,0 );
    layout->addWidget( imageInfoLabel, 2,0 );

    parent->setLayout(layout);
}

void QmitkDataManagerLightView::SetFocus()
{
}

void QmitkDataManagerLightView::on_DataItemList_currentRowChanged(int currentRow)
{
}
