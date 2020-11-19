/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRemeshingView.h"

#include <ui_QmitkRemeshingViewControls.h>

#include <berryQtStyleManager.h>

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkRemeshing.h>

const std::string QmitkRemeshingView::VIEW_ID = "org.mitk.views.remeshing";

QmitkRemeshingView::QmitkRemeshingView()
  : m_Controls(new Ui::QmitkRemeshingViewControls)
{
}

QmitkRemeshingView::~QmitkRemeshingView()
{
}

void QmitkRemeshingView::CreateQtPartControl(QWidget* parent)
{
  m_Controls->setupUi(parent);

  m_Controls->decimatePushButton->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/Remeshing/RemeshingIcon.svg")));

  m_Controls->selectionWidget->SetDataStorage(this->GetDataStorage());
  m_Controls->selectionWidget->SetSelectionIsOptional(true);
  m_Controls->selectionWidget->SetEmptyInfo(QStringLiteral("Select a surface"));
  m_Controls->selectionWidget->SetAutoSelectNewNodes(true);
  m_Controls->selectionWidget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Surface>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
      mitk::NodePredicateProperty::New("helper object"),
      mitk::NodePredicateProperty::New("hidden object")))));

  connect(m_Controls->selectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkRemeshingView::OnSurfaceChanged);
  connect(m_Controls->vertexCountSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVertexCountChanged(int)));
  connect(m_Controls->vertexCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnVertexCountChanged(int)));
  connect(m_Controls->calculateNormalsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnCalculateNormalsChanged(int)));
  connect(m_Controls->decimatePushButton, SIGNAL(clicked()), this, SLOT(OnDecimateButtonClicked()));

  this->OnSurfaceChanged(m_Controls->selectionWidget->GetSelectedNodes());
}

void QmitkRemeshingView::OnSurfaceChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes)
{
  this->EnableWidgets(!nodes.empty() && nodes.front().IsNotNull());
}

void QmitkRemeshingView::OnVertexCountChanged(int vertexCount)
{
  if (vertexCount != m_Controls->vertexCountSlider->value())
    m_Controls->vertexCountSlider->setValue(vertexCount);

  if (vertexCount != m_Controls->vertexCountSpinBox->value())
    m_Controls->vertexCountSpinBox->setValue(vertexCount);
}

void QmitkRemeshingView::OnCalculateNormalsChanged(int checkState)
{
  m_Controls->flipNormalsCheckBox->setEnabled(Qt::Unchecked != checkState);
}

void QmitkRemeshingView::OnDecimateButtonClicked()
{
  mitk::DataNode::Pointer selectedNode = m_Controls->selectionWidget->GetSelectedNode();
  mitk::Surface::ConstPointer input = static_cast<mitk::Surface*>(selectedNode->GetData());
  mitk::Surface::Pointer output;

  try
  {
    output = mitk::Remeshing::Decimate(input,
      0.01 * m_Controls->vertexCountSpinBox->value(),
      m_Controls->calculateNormalsCheckBox->isChecked(),
      m_Controls->flipNormalsCheckBox->isChecked());
  }
  catch(const mitk::Exception& exception)
  {
    MITK_ERROR << exception.GetDescription();
    return;
  }

  if (output.IsNull())
    return;

  auto newNode = mitk::DataNode::New();
  newNode->SetName(QString("%1 (decimated)").arg(selectedNode->GetName().c_str()).toStdString());
  newNode->SetData(output);

  this->GetDataStorage()->Add(newNode, selectedNode);
}

void QmitkRemeshingView::EnableWidgets(bool enable)
{
  m_Controls->vertexCountSlider->setEnabled(enable);
  m_Controls->vertexCountSpinBox->setEnabled(enable);
  m_Controls->calculateNormalsCheckBox->setEnabled(enable);
  m_Controls->flipNormalsCheckBox->setEnabled(enable && m_Controls->calculateNormalsCheckBox->isChecked());
  m_Controls->decimatePushButton->setEnabled(enable);
}

void QmitkRemeshingView::SetFocus()
{
  m_Controls->selectionWidget->setFocus();
}

