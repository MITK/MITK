/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPointSetInteractionView.h"

#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>
#include <QInputDialog>
#include <QLineEdit>

#include <mitkDataNodeObject.h>
#include <mitkDataNodeSelection.h>
#include <mitkProperties.h>

#include <QmitkPointListWidget.h>
#include <QmitkRenderWindow.h>

const std::string QmitkPointSetInteractionView::VIEW_ID = "org.mitk.views.pointsetinteraction";

QmitkPointSetInteractionView::QmitkPointSetInteractionView()
  : m_Controls(nullptr)
{
}

QmitkPointSetInteractionView::~QmitkPointSetInteractionView()
{
}

void QmitkPointSetInteractionView::CreateQtPartControl(QWidget *parent)
{
  m_Controls = new Ui::QmitkPointSetInteractionViewControls;
  m_Controls->setupUi(parent);

  connect(m_Controls->addPointSetPushButton, &QPushButton::clicked,
    this, &QmitkPointSetInteractionView::OnAddPointSetClicked);

  if (mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart())
  {
    RenderWindowPartActivated(renderWindowPart);
  }
}

void QmitkPointSetInteractionView::SetFocus()
{
  m_Controls->addPointSetPushButton->setFocus();
}

void QmitkPointSetInteractionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  mitk::DataNode::Pointer selectedNode;

  if (!nodes.empty())
  {
    selectedNode = nodes.front();
  }

  mitk::PointSet::Pointer pointSet;

  if (selectedNode.IsNotNull())
  {
    pointSet = dynamic_cast<mitk::PointSet*>(selectedNode->GetData());
  }

  if (pointSet.IsNotNull())
  {
    m_SelectedPointSetNode = selectedNode;
    m_Controls->currentPointSetLabel->setText(QString::fromStdString(selectedNode->GetName()));
    m_Controls->poinSetListWidget->SetPointSetNode(selectedNode);
  }
  else
  {
    m_Controls->currentPointSetLabel->setText(tr("None"));
    m_Controls->poinSetListWidget->SetPointSetNode(nullptr);
  }
}

void QmitkPointSetInteractionView::NodeChanged(const mitk::DataNode* node)
{
  if (node == m_SelectedPointSetNode && m_Controls->currentPointSetLabel->text().toStdString() != node->GetName())
  {
    m_Controls->currentPointSetLabel->setText(QString::fromStdString(node->GetName()));
  }
}

void QmitkPointSetInteractionView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (nullptr != m_Controls)
  {
    m_Controls->poinSetListWidget->AddSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    m_Controls->poinSetListWidget->AddSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    m_Controls->poinSetListWidget->AddSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
  }
}

void QmitkPointSetInteractionView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (nullptr != m_Controls)
  {
    m_Controls->poinSetListWidget->RemoveSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    m_Controls->poinSetListWidget->RemoveSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    m_Controls->poinSetListWidget->RemoveSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
  }
}

void QmitkPointSetInteractionView::OnAddPointSetClicked()
{
  // ask for the name of the point set
  bool ok = false;
  QString name = QInputDialog::getText(QApplication::activeWindow(),
    tr("Add point set..."), tr("Enter name for the new point set"), QLineEdit::Normal, tr("PointSet"), &ok);
  if (!ok || name.isEmpty())
  {
    return;
  }

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  pointSetNode->SetData(pointSet);
  pointSetNode->SetProperty("name", mitk::StringProperty::New(name.toStdString()));
  pointSetNode->SetProperty("opacity", mitk::FloatProperty::New(1));
  pointSetNode->SetColor(1.0, 1.0, 0.0);
  this->GetDataStorage()->Add(pointSetNode);

  // create a new selection and emulate selection for this
  berry::IWorkbenchPart::Pointer nullPart;
  QList<mitk::DataNode::Pointer> selection;
  selection.push_back(pointSetNode);
  this->OnSelectionChanged(nullPart, selection);
}
