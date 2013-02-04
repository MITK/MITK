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
    QIcon m_ItemIcon;

    // data
    QList<mitk::DataNode*> m_DataNodes;
    int m_CurrentIndex;

    // widget
    QListWidget* m_ListWidget;
    QLabel* m_ImageInfoLabel;

};

QmitkDataManagerLightView::QmitkDataManagerLightView()
    : d( new QmitkDataManagerLightViewData )
{
    d->m_Predicate = mitk::NodePredicateDataType::New("Image");
    d->m_ItemIcon = QIcon(":/org.mitk.gui.qt.datamanagerlight/Image_24.png");
    d->m_CurrentIndex = -1;
    d->m_ListWidget = 0;
    d->m_ImageInfoLabel = 0;
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
        d->m_ListWidget->addItem( new QListWidgetItem( d->m_ItemIcon, QString::fromStdString( node->GetName() ) ) );
    }
}

void QmitkDataManagerLightView::NodeRemoved(const mitk::DataNode *node)
{
    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    int index = d->m_DataNodes.indexOf(nonConstNode);
    if( index >= 0 )
    {
        MITK_INFO << "removing node at: " << index;
        QListWidgetItem* item = d->m_ListWidget->takeItem(index);
        delete item;

        d->m_DataNodes.removeAt(index);
        MITK_INFO << "item deleted";
    }
}

void QmitkDataManagerLightView::CreateQtPartControl(QWidget* parent)
{

    QPushButton* loadButton = new QPushButton("Load image");

    d->m_ListWidget = new QListWidget;

    d->m_ImageInfoLabel = new QLabel;

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( loadButton, 0,0 );
    layout->addWidget( d->m_ImageInfoLabel, 1,0 );
    layout->addWidget( d->m_ListWidget, 2,0 );

    parent->setLayout(layout);

    connect(d->m_ListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(on_DataItemList_currentRowChanged(int)) );
    this->EvaluateLabelString();
}

void QmitkDataManagerLightView::SetFocus()
{
}

void QmitkDataManagerLightView::on_DataItemList_currentRowChanged(int currentRow)
{
    MITK_INFO << "DataItemList currentRowChanged: " << currentRow;
    Q_UNUSED(currentRow)

    this->EvaluateLabelString();
}

void QmitkDataManagerLightView::EvaluateLabelString()
{
    d->m_CurrentIndex = d->m_ListWidget->currentRow();
    MITK_INFO << "the currently selected index: " << d->m_CurrentIndex;

    QString newLabelText = "Current patient: ";
    if( d->m_CurrentIndex >= 0 )
    {
        // TODO WHERE IS THE PATIENT NAME?
        std::string name = d->m_DataNodes.at(d->m_CurrentIndex)->GetName();
        newLabelText.append( QString("<strong>%1</strong>" ).arg( QString::fromStdString(name) ) );
    }
    else
    {
        newLabelText.append("<strong>Unknown</strong>");
    }
    d->m_ImageInfoLabel->setText(newLabelText);
}
