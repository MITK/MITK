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
#include <QApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <mitkCoreObjectFactory.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIRenderingManager.h>
#include <mitkIOUtil.h>

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
    QPushButton* m_RemoveButton;

};

QmitkDataManagerLightView::QmitkDataManagerLightView()
    : d( new QmitkDataManagerLightViewData )
{
    d->m_Predicate = mitk::NodePredicateDataType::New("Image");
    d->m_ItemIcon = QIcon(":/org.mitk.gui.qt.datamanagerlight/Image_24.png");
    d->m_CurrentIndex = -1;
    d->m_ListWidget = 0;
    d->m_ImageInfoLabel = 0;
    d->m_RemoveButton = 0;
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
    this->RemoveNode( const_cast<mitk::DataNode*>(node) );
}

void QmitkDataManagerLightView::NodeChanged(const mitk::DataNode *node)
{
    MITK_DEBUG << "NodeChanged";
    if( d->m_DataNodes.contains(const_cast<mitk::DataNode*>(node)) )
        this->ToggleVisibility();
}

void QmitkDataManagerLightView::RemoveNode(mitk::DataNode *node)
{
    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    int index = d->m_DataNodes.indexOf(nonConstNode);
    if( index >= 0 )
    {
        MITK_DEBUG << "removing node at: " << index;
        QListWidgetItem* item = d->m_ListWidget->takeItem(index);
        delete item;

        d->m_DataNodes.removeAt(index);
        MITK_DEBUG << "item deleted";
    }
}

void QmitkDataManagerLightView::CreateQtPartControl(QWidget* parent)
{

    QPushButton* loadButton = new QPushButton(QIcon(":/org.mitk.gui.qt.datamanagerlight/Load_48.png"), "Load");
    d->m_RemoveButton = new QPushButton(QIcon(":/org.mitk.gui.qt.datamanagerlight/Remove_48.png"), "Remove");
    d->m_RemoveButton->setEnabled(false);

    d->m_ListWidget = new QListWidget;

    d->m_ImageInfoLabel = new QLabel;

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( loadButton, 0,0 );
    layout->addWidget( d->m_RemoveButton, 0,1 );
    layout->addWidget( d->m_ImageInfoLabel, 1,0, 1, 2 );
    layout->addWidget( d->m_ListWidget, 2,0,1,2 );

    parent->setLayout(layout);

    connect(d->m_ListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(on_DataItemList_currentRowChanged(int)) );
    connect(loadButton, SIGNAL(pressed()), this, SLOT(on_Load_pressed()) );
    connect(d->m_RemoveButton, SIGNAL(pressed()), this, SLOT(on_Remove_pressed()) );

    this->ListSelectionChanged();
}

void QmitkDataManagerLightView::SetFocus()
{
    d->m_ListWidget->setFocus();
}

void QmitkDataManagerLightView::on_DataItemList_currentRowChanged(int currentRow)
{
    MITK_DEBUG << "DataItemList currentRowChanged: " << currentRow;
    Q_UNUSED(currentRow)

    this->ListSelectionChanged();
}

void QmitkDataManagerLightView::ListSelectionChanged()
{
    d->m_CurrentIndex = d->m_ListWidget->currentRow();
    MITK_DEBUG << "the currently selected index: " << d->m_CurrentIndex;

    QString newLabelText = "Current patient: ";
    if( d->m_CurrentIndex >= 0 )
    {
        // TODO WHERE IS THE PATIENT NAME?
        std::string name = d->m_DataNodes.at(d->m_CurrentIndex)->GetName();
        newLabelText.append( QString("<strong>%1</strong>" ).arg( QString::fromStdString(name) ) );
        d->m_RemoveButton->setEnabled(true);
    }
    else
    {
        newLabelText.append("<strong>Unknown</strong>");
        d->m_RemoveButton->setEnabled(false);
    }
    d->m_ImageInfoLabel->setText(newLabelText);
    this->ToggleVisibility();
}

void QmitkDataManagerLightView::on_Load_pressed()
{
    MITK_DEBUG << "on_Load_pressed";
  QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Load data", "", mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    FileOpen((*it).toLatin1(), 0);
  }
}

void QmitkDataManagerLightView::FileOpen( const char * fileName, mitk::DataNode* /*parentNode*/ )
{
  try
  {
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    mitk::IOUtil::Load(fileName, *this->GetDataStorage());
    mitk::RenderingManager::GetInstance()->InitializeViews();
  }
  catch ( itk::ExceptionObject & ex )
  {
    MITK_ERROR << "Exception during file open: " << ex;
  }

  QApplication::restoreOverrideCursor();
}

void QmitkDataManagerLightView::on_Remove_pressed()
{
    d->m_CurrentIndex = d->m_ListWidget->currentRow();
    MITK_DEBUG << "the currently selected index: " << d->m_CurrentIndex;

    mitk::DataNode* node = d->m_DataNodes.at(d->m_CurrentIndex);
    QString question = tr("Do you really want to remove ");
    // TODO patient name?
    question.append( QString::fromStdString( node->GetName() ) );
    question.append(" ?");

    QMessageBox::StandardButton answerButton = QMessageBox::question( NULL
      , tr("DataManagerLight")
      , question
      , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if(answerButton == QMessageBox::Yes)
    {
        this->GetDataStorage()->Remove(node);
        this->GlobalReinit();
    }
}

void QmitkDataManagerLightView::GlobalReinit()
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  // no render window available
  if (renderWindow == NULL) return;

  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNot::Pointer pred
    = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
    , mitk::BoolProperty::New(false)));

  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  // calculate bounding geometry of these nodes
  mitk::TimeGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  renderWindow->GetRenderingManager()->InitializeViews(bounds);
}

void QmitkDataManagerLightView::ToggleVisibility()
{
    bool changedAnything = false;
    bool isVisible = false;
    for(size_t i=0; i<d->m_DataNodes.size(); ++i)
    {
        isVisible = false;
        d->m_DataNodes.at(i)->GetVisibility(isVisible, 0 );

        if( d->m_CurrentIndex == i && isVisible == false )
        {
            d->m_DataNodes.at(i)->SetVisibility(true);
            changedAnything = true;
        }
        else if( d->m_CurrentIndex != i && isVisible == true )
        {
            d->m_DataNodes.at(i)->SetVisibility(false);
            changedAnything = true;
        }
    }

    if( changedAnything )
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
