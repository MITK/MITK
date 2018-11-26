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

#include "QmitkRemeshingView.h"

#include <berryQtStyleManager.h>

#include <mitkACVD.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkSurface.h>
#include <mitkVtkRepresentationProperty.h>

#include <vtkPolyData.h>
#include <vtkProperty.h>

QmitkRemeshingView::QmitkRemeshingView()
{
}

QmitkRemeshingView::~QmitkRemeshingView()
{
}

void QmitkRemeshingView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.remeshPushButton->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/Remeshing/RemeshingIcon.svg")));

  m_Controls.surfaceComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.surfaceComboBox->SetPredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Surface>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
      mitk::NodePredicateProperty::New("helper object"),
      mitk::NodePredicateProperty::New("hidden object")))));

  connect(m_Controls.surfaceComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSelectedSurfaceChanged(const mitk::DataNode *)));
  connect(m_Controls.densitySlider, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
  connect(m_Controls.densitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
  connect(m_Controls.remeshPushButton, SIGNAL(clicked()), this, SLOT(OnRemeshButtonClicked()));

  this->OnSelectedSurfaceChanged(m_Controls.surfaceComboBox->GetSelectedNode());
}

void QmitkRemeshingView::OnSelectedSurfaceChanged(const mitk::DataNode *node)
{
  if (node != nullptr)
  {
    m_MaxNumberOfVertices = static_cast<int>(static_cast<mitk::Surface*>(node->GetData())->GetVtkPolyData()->GetNumberOfPoints());
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
  if (density != m_Controls.densitySlider->value())
    m_Controls.densitySlider->setValue(density);

  if (density != m_Controls.densitySpinBox->value())
    m_Controls.densitySpinBox->setValue(density);
}

void QmitkRemeshingView::OnRemeshButtonClicked()
{
  mitk::DataNode::Pointer selectedNode = m_Controls.surfaceComboBox->GetSelectedNode();
  mitk::Surface::ConstPointer surface = static_cast<mitk::Surface*>(selectedNode->GetData());

  int density = m_Controls.densitySpinBox->value();
  int numVertices = std::max(100, static_cast<int>(m_MaxNumberOfVertices * (density * 0.01)));

  double gradation = m_Controls.remeshingComboBox->currentText() == QStringLiteral("Adaptive")
    ? 1.0
    : 0.0;

  const QString quality = m_Controls.qualityComboBox->currentText();
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

  bool boundaryFixing = m_Controls.preserveEdgesCheckBox->isChecked();

  mitk::ACVD::RemeshFilter::Pointer remesher = mitk::ACVD::RemeshFilter::New();
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

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(QString("%1 (%2%)").arg(selectedNode->GetName().c_str()).arg(density).toStdString());
  newNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  newNode->SetData(remeshedSurface);

  this->GetDataStorage()->Add(newNode, selectedNode);
}

void QmitkRemeshingView::EnableWidgets(bool enable)
{
  m_Controls.surfaceComboBox->setEnabled(enable);
  m_Controls.densitySlider->setEnabled(enable);
  m_Controls.densitySpinBox->setEnabled(enable);
  m_Controls.remeshingComboBox->setEnabled(enable);
  m_Controls.qualityComboBox->setEnabled(enable);
  m_Controls.preserveEdgesCheckBox->setEnabled(enable);
  m_Controls.remeshPushButton->setEnabled(enable);

  m_Controls.explanationLabel->setVisible(enable);
}

void QmitkRemeshingView::SetFocus()
{
  m_Controls.surfaceComboBox->setFocus();
}

