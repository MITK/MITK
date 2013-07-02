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
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkACVD.h>
#include <mitkSurface.h>
#include <vtkPolyData.h>
#include <algorithm>

QmitkRemeshingView::QmitkRemeshingView()
{
}

QmitkRemeshingView::~QmitkRemeshingView()
{
}

void QmitkRemeshingView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.surfaceComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.surfaceComboBox->SetPredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Surface>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));

  this->EnableWidgets(m_Controls.surfaceComboBox->GetSelectedNode().IsNotNull());

  connect(m_Controls.surfaceComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSelectedSurfaceChanged(const mitk::DataNode *)));
  connect(m_Controls.numVerticesSlider, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfVerticesChanged(int)));
  connect(m_Controls.numVerticesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfVerticesChanged(int)));
  connect(m_Controls.gradationSlider, SIGNAL(valueChanged(double)), this, SLOT(OnGradationChanged(double)));
  connect(m_Controls.gradationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnGradationChanged(double)));
  connect(m_Controls.remeshPushButton, SIGNAL(clicked()), this, SLOT(OnRemeshButtonClicked()));
}

void QmitkRemeshingView::EnableWidgets(bool enable)
{
  m_Controls.surfaceComboBox->setEnabled(enable);
  m_Controls.numVerticesSlider->setEnabled(enable);
  m_Controls.numVerticesSpinBox->setEnabled(enable);
  m_Controls.gradationSlider->setEnabled(enable);
  m_Controls.gradationSpinBox->setEnabled(enable);
  m_Controls.boundaryFixingCheckBox->setEnabled(enable);
  m_Controls.remeshPushButton->setEnabled(enable);
}

void QmitkRemeshingView::OnGradationChanged(double gradation)
{
  if (gradation != m_Controls.gradationSlider->value())
    m_Controls.gradationSlider->setValue(gradation);

  if (gradation != m_Controls.gradationSpinBox->value())
    m_Controls.gradationSpinBox->setValue(gradation);
}

void QmitkRemeshingView::OnNumberOfVerticesChanged(int numVertices)
{
  if (numVertices != m_Controls.numVerticesSlider->value())
    m_Controls.numVerticesSlider->setValue(numVertices);

  if (numVertices != m_Controls.numVerticesSpinBox->value())
    m_Controls.numVerticesSpinBox->setValue(numVertices);
}

void QmitkRemeshingView::OnRemeshButtonClicked()
{
  mitk::DataNode::Pointer selectedNode = m_Controls.surfaceComboBox->GetSelectedNode();
  mitk::Surface::Pointer surface = static_cast<mitk::Surface*>(selectedNode->GetData());
  int numVertices = m_Controls.numVerticesSpinBox->value();
  double gradation = m_Controls.gradationSpinBox->value();
  bool boundaryFixing = m_Controls.boundaryFixingCheckBox->isChecked();

  mitk::Surface::Pointer remeshedSurface = mitk::ACVD::Remesh(surface, numVertices, gradation, boundaryFixing);

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(QString("%1 (%2, %3)").arg(selectedNode->GetName().c_str()).arg(remeshedSurface->GetVtkPolyData()->GetNumberOfPoints()).arg(gradation).toStdString());
  newNode->SetFloatProperty("material.specularCoefficient", 0.0f);
  newNode->SetData(remeshedSurface);

  this->GetDataStorage()->Add(newNode, selectedNode);
}

void QmitkRemeshingView::OnSelectedSurfaceChanged(const mitk::DataNode *node)
{
  if (node != NULL)
  {
    int numVertices = static_cast<int>(static_cast<mitk::Surface*>(node->GetData())->GetVtkPolyData()->GetNumberOfPoints());
    this->SetNumberOfVertices(10, 2 * numVertices, std::max(1, numVertices / 10), numVertices);
    this->EnableWidgets(true);
  }
  else
  {
    this->EnableWidgets(false);
    this->SetNumberOfVertices(0, 0, 0, 0);
  }
}

void QmitkRemeshingView::SetFocus()
{
  m_Controls.surfaceComboBox->setFocus();
}

void QmitkRemeshingView::SetNumberOfVertices(int minimum, int maximum, int step, int value)
{
  m_Controls.numVerticesSlider->setMinimum(minimum);
  m_Controls.numVerticesSlider->setMaximum(maximum);
  m_Controls.numVerticesSlider->setSingleStep(1);
  m_Controls.numVerticesSlider->setPageStep(step);

  m_Controls.numVerticesSpinBox->setMinimum(minimum);
  m_Controls.numVerticesSpinBox->setMaximum(maximum);
  m_Controls.numVerticesSpinBox->setSingleStep(step);
  m_Controls.numVerticesSpinBox->setValue(value);
}
