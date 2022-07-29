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
#include <mitkVtkRepresentationProperty.h>

#include <vtkPolyData.h>
#include <vtkProperty.h>

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

  m_Controls->remeshPushButton->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/Remeshing/RemeshingIcon.svg")));

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
  connect(m_Controls->densitySlider, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
  connect(m_Controls->densitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
  connect(m_Controls->remeshPushButton, SIGNAL(clicked()), this, SLOT(OnRemeshButtonClicked()));

  this->OnSurfaceChanged(m_Controls->selectionWidget->GetSelectedNodes());
}

void QmitkRemeshingView::OnSurfaceChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes)
{
  if (!nodes.empty() && nodes.front().IsNotNull())
  {
    m_MaxNumberOfVertices = static_cast<int>(static_cast<mitk::Surface*>(nodes.front()->GetData())->GetVtkPolyData()->GetNumberOfPoints());
    this->EnableWidgets(true);
  }
  else
  {
    m_MaxNumberOfVertices = 0;
    this->EnableWidgets(false);
  }
}

void QmitkRemeshingView::OnDensityChanged(int density)
{
  if (density != m_Controls->densitySlider->value())
    m_Controls->densitySlider->setValue(density);

  if (density != m_Controls->densitySpinBox->value())
    m_Controls->densitySpinBox->setValue(density);
}

void QmitkRemeshingView::OnRemeshButtonClicked()
{
  auto selectedNode = m_Controls->selectionWidget->GetSelectedNode();
  mitk::Surface::ConstPointer surface = static_cast<mitk::Surface*>(selectedNode->GetData());

  int density = m_Controls->densitySpinBox->value();
  int numVertices = std::max(100, static_cast<int>(m_MaxNumberOfVertices * (density * 0.01)));

  double gradation = m_Controls->remeshingComboBox->currentText() == QStringLiteral("Adaptive")
    ? 1.0
    : 0.0;

  const QString quality = m_Controls->qualityComboBox->currentText();
  int subsampling;

  if (QStringLiteral("High (slow)") == quality)
  {
    subsampling = 50;
  }
  else if (QStringLiteral("Maximum (very slow)") == quality)
  {
    subsampling = 500;
  }
  else // The default is "Medium (fast)".
  {
    subsampling = 10;
  }

  bool boundaryFixing = m_Controls->preserveEdgesCheckBox->isChecked();

  auto remesher = mitk::RemeshFilter::New();
  remesher->SetInput(surface);
  remesher->SetTimeStep(0);
  remesher->SetNumVertices(numVertices);
  remesher->SetGradation(gradation);
  remesher->SetSubsampling(subsampling);
  remesher->SetEdgeSplitting(0.0);
  remesher->SetOptimizationLevel(1.0);
  remesher->SetForceManifold(false);
  remesher->SetBoundaryFixing(boundaryFixing);

  try
  {
    remesher->Update();
  }
  catch(const mitk::Exception& exception)
  {
    MITK_ERROR << exception.GetDescription();
    return;
  }

  mitk::Surface::Pointer remeshedSurface = remesher->GetOutput();

  auto newNode = mitk::DataNode::New();
  newNode->SetName(QString("%1 (%2%)").arg(selectedNode->GetName().c_str()).arg(density).toStdString());
  newNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  newNode->SetData(remeshedSurface);

  this->GetDataStorage()->Add(newNode, selectedNode);
}

void QmitkRemeshingView::EnableWidgets(bool enable)
{
  m_Controls->densitySlider->setEnabled(enable);
  m_Controls->densitySpinBox->setEnabled(enable);
  m_Controls->remeshingComboBox->setEnabled(enable);
  m_Controls->qualityComboBox->setEnabled(enable);
  m_Controls->preserveEdgesCheckBox->setEnabled(enable);
  m_Controls->remeshPushButton->setEnabled(enable);

  m_Controls->explanationLabel->setVisible(enable);
}

void QmitkRemeshingView::SetFocus()
{
  m_Controls->selectionWidget->setFocus();
}
