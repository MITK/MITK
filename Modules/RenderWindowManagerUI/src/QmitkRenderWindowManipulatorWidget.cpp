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

// render window manager UI module
#include "QmitkRenderWindowManipulatorWidget.h"

#include "QmitkCustomVariants.h"

// mitk core
#include <mitkBaseRenderer.h>

// qt
#include <QSignalMapper>

QmitkRenderWindowManipulatorWidget::QmitkRenderWindowManipulatorWidget(mitk::DataStorage::Pointer dataStorage, QWidget* parent /*=nullptr*/)
  : QWidget(parent)
  , m_DataStorage(dataStorage)
{
  Init();
}

void QmitkRenderWindowManipulatorWidget::Init()
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  // initialize the render window layer controller and the render window view direction controller
  // and set the controller renderer (in constructor) and the data storage
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowLayerController->SetDataStorage(m_DataStorage);
  m_RenderWindowViewDirectionController->SetDataStorage(m_DataStorage);

  // create a new model
  m_RenderWindowDataModel = std::make_unique<QmitkRenderWindowDataModel>(this);
  m_RenderWindowDataModel->SetDataStorage(m_DataStorage);

  m_Controls.renderWindowTableView->setModel(m_RenderWindowDataModel.get());
  m_Controls.renderWindowTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowTableView->horizontalHeader()->setStretchLastSection(true);
  m_Controls.renderWindowTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.renderWindowTableView->setSelectionMode(QAbstractItemView::SingleSelection);

  SetUpConnections();
}

void QmitkRenderWindowManipulatorWidget::SetUpConnections()
{
  // signal to signal connection
  connect(m_Controls.pushButtonAddLayer, SIGNAL(clicked()), this, SIGNAL(AddLayerButtonClicked()));
  connect(m_Controls.pushButtonRemoveLayer, SIGNAL(clicked()), this, SLOT(RemoveLayer()));
  connect(m_Controls.pushButtonSetAsBaseLayer, SIGNAL(clicked()), this, SLOT(SetAsBaseLayer()));

  QSignalMapper* udSignalMapper = new QSignalMapper(this);
  udSignalMapper->setMapping(m_Controls.pushButtonMoveUp, QString("up"));
  udSignalMapper->setMapping(m_Controls.pushButtonMoveDown, QString("down"));
  connect(udSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(MoveLayer(const QString&)));
  connect(m_Controls.pushButtonMoveUp, SIGNAL(clicked()), udSignalMapper, SLOT(map()));
  connect(m_Controls.pushButtonMoveDown, SIGNAL(clicked()), udSignalMapper, SLOT(map()));

  connect(m_Controls.pushButtonResetRenderer, SIGNAL(clicked()), this, SLOT(ResetRenderer()));
  connect(m_Controls.pushButtonClearRenderer, SIGNAL(clicked()), this, SLOT(ClearRenderer()));

  QSignalMapper* changeViewDirectionSignalMapper = new QSignalMapper(this);
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonAxial, QString("axial"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonCoronal, QString("coronal"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonSagittal, QString("sagittal"));
  connect(changeViewDirectionSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(ChangeViewDirection(const QString&)));
  connect(m_Controls.radioButtonAxial, SIGNAL(clicked()), changeViewDirectionSignalMapper, SLOT(map()));
  connect(m_Controls.radioButtonCoronal, SIGNAL(clicked()), changeViewDirectionSignalMapper, SLOT(map()));
  connect(m_Controls.radioButtonSagittal, SIGNAL(clicked()), changeViewDirectionSignalMapper, SLOT(map()));
}

void QmitkRenderWindowManipulatorWidget::SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_RenderWindowLayerController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
}

void QmitkRenderWindowManipulatorWidget::SetActiveRenderWindow(const QString &renderWindowId)
{
  std::string currentRendererName = renderWindowId.toStdString();
  m_RenderWindowDataModel->SetCurrentRenderer(currentRendererName);

  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(currentRendererName);
  if (nullptr == selectedRenderer)
  {
    return;
  }

  mitk::SliceNavigationController::ViewDirection viewDirection = selectedRenderer->GetSliceNavigationController()->GetDefaultViewDirection();
  switch (viewDirection)
  {
  case mitk::SliceNavigationController::Axial:
    m_Controls.radioButtonAxial->setChecked(true);
    break;
  case mitk::SliceNavigationController::Frontal:
    m_Controls.radioButtonCoronal->setChecked(true);
    break;
  case mitk::SliceNavigationController::Sagittal:
    m_Controls.radioButtonSagittal->setChecked(true);
    break;
  default:
    break;
  }
}

void QmitkRenderWindowManipulatorWidget::AddLayer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, m_RenderWindowDataModel->GetCurrentRenderer());
}

void QmitkRenderWindowManipulatorWidget::AddLayerToAllRenderer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, nullptr);
}

void QmitkRenderWindowManipulatorWidget::HideDataNodeInAllRenderer(const mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->HideDataNodeInAllRenderer(dataNode);
}

void QmitkRenderWindowManipulatorWidget::RemoveLayer()
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {

    QVariant qvariantDataNode = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      m_RenderWindowLayerController->RemoveLayerNode(dataNode, m_RenderWindowDataModel->GetCurrentRenderer());
      m_Controls.renderWindowTableView->clearSelection();
    }
  }
}

void QmitkRenderWindowManipulatorWidget::SetAsBaseLayer()
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      m_RenderWindowLayerController->SetBaseDataNode(dataNode, m_RenderWindowDataModel->GetCurrentRenderer());
      m_Controls.renderWindowTableView->clearSelection();
    }
  }
}

void QmitkRenderWindowManipulatorWidget::MoveLayer(const QString &direction)
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      const mitk::BaseRenderer* selectedRenderer = m_RenderWindowDataModel->GetCurrentRenderer();

      bool success = false;
      if ("up" == direction)
      {
        success = m_RenderWindowLayerController->MoveNodeUp(dataNode, selectedRenderer);
        if (success)
        {
          // node has been successfully moved up
          m_Controls.renderWindowTableView->selectRow(selectedIndex.row() - 1);
        }
      }
      else
      {
        success = m_RenderWindowLayerController->MoveNodeDown(dataNode, selectedRenderer);
        if (success)
        {
          // node has been successfully moved down
          m_Controls.renderWindowTableView->selectRow(selectedIndex.row() + 1);
        }
      }
    }
  }
}

void QmitkRenderWindowManipulatorWidget::ResetRenderer()
{
  m_RenderWindowLayerController->ResetRenderer(true, m_RenderWindowDataModel->GetCurrentRenderer());
  m_Controls.renderWindowTableView->clearSelection();
}

void QmitkRenderWindowManipulatorWidget::ClearRenderer()
{
  m_RenderWindowLayerController->ResetRenderer(false, m_RenderWindowDataModel->GetCurrentRenderer());
  m_Controls.renderWindowTableView->clearSelection();
}

void QmitkRenderWindowManipulatorWidget::ChangeViewDirection(const QString &viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), m_RenderWindowDataModel->GetCurrentRenderer());
}
