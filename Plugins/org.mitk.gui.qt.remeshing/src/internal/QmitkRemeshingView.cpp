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

// MITK
#include <mitkACVD.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkSurface.h>
#include <mitkVtkRepresentationProperty.h>

// Qt
#include <QIntValidator>

// VTK
#include <vtkPolyData.h>
#include <vtkProperty.h>

// C++ Standard Library
#include <algorithm>
#include <limits>

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

  QIntValidator* posIntValidator = new QIntValidator(0, std::numeric_limits<int>::max(), parent);
  m_Controls.maxNumVerticesLineEdit->setValidator(posIntValidator);

  this->EnableWidgets(m_Controls.surfaceComboBox->GetSelectedNode().IsNotNull());
  this->OnAdvancedSettingsButtonToggled(false);

  connect(m_Controls.surfaceComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSelectedSurfaceChanged(const mitk::DataNode *)));
  connect(m_Controls.numVerticesSlider, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfVerticesChanged(int)));
  connect(m_Controls.numVerticesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfVerticesChanged(int)));
  connect(m_Controls.gradationSlider, SIGNAL(valueChanged(double)), this, SLOT(OnGradationChanged(double)));
  connect(m_Controls.gradationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnGradationChanged(double)));
  connect(m_Controls.advancedSettingsButton, SIGNAL(toggled(bool)), this, SLOT(OnAdvancedSettingsButtonToggled(bool)));
  connect(m_Controls.maxNumVerticesLineEdit, SIGNAL(editingFinished()), this, SLOT(OnMaxNumVerticesLineEditEditingFinished()));
  connect(m_Controls.edgeSplittingSlider, SIGNAL(valueChanged(double)), this, SLOT(OnEdgeSplittingChanged(double)));
  connect(m_Controls.edgeSplittingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnEdgeSplittingChanged(double)));
  connect(m_Controls.subsamplingSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSubsamplingChanged(int)));
  connect(m_Controls.subsamplingSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSubsamplingChanged(int)));
  connect(m_Controls.optimizationLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(OnOptimizationLevelChanged(int)));
  connect(m_Controls.optimizationLevelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnOptimizationLevelChanged(int)));
  connect(m_Controls.remeshPushButton, SIGNAL(clicked()), this, SLOT(OnRemeshButtonClicked()));

  this->OnSelectedSurfaceChanged(m_Controls.surfaceComboBox->GetSelectedNode());
}

void QmitkRemeshingView::EnableWidgets(bool enable)
{
  m_Controls.surfaceComboBox->setEnabled(enable);
  m_Controls.numVerticesSlider->setEnabled(enable);
  m_Controls.numVerticesSpinBox->setEnabled(enable);
  m_Controls.gradationSlider->setEnabled(enable);
  m_Controls.gradationSpinBox->setEnabled(enable);
  m_Controls.maxNumVerticesLineEdit->setEnabled(enable);
  m_Controls.edgeSplittingSlider->setEnabled(enable);
  m_Controls.edgeSplittingSpinBox->setEnabled(enable);
  m_Controls.subsamplingSlider->setEnabled(enable);
  m_Controls.subsamplingSpinBox->setEnabled(enable);
  m_Controls.optimizationLevelSlider->setEnabled(enable);
  m_Controls.optimizationLevelSpinBox->setEnabled(enable);
  m_Controls.forceManifoldCheckBox->setEnabled(enable);
  m_Controls.boundaryFixingCheckBox->setEnabled(enable);
  m_Controls.remeshPushButton->setEnabled(enable);
}

void QmitkRemeshingView::OnAdvancedSettingsButtonToggled(bool toggled)
{
  m_Controls.maxNumVerticesLabel->setVisible(toggled);
  m_Controls.maxNumVerticesLineEdit->setVisible(toggled);
  m_Controls.edgeSplittingLabel->setVisible(toggled);
  m_Controls.edgeSplittingSlider->setVisible(toggled);
  m_Controls.edgeSplittingSpinBox->setVisible(toggled);
  m_Controls.subsamplingLabel->setVisible(toggled);
  m_Controls.subsamplingSlider->setVisible(toggled);
  m_Controls.subsamplingSpinBox->setVisible(toggled);
  m_Controls.optimizationLevelLabel->setVisible(toggled);
  m_Controls.optimizationLevelSlider->setVisible(toggled);
  m_Controls.optimizationLevelSpinBox->setVisible(toggled);
  m_Controls.forceManifoldCheckBox->setVisible(toggled);
  m_Controls.boundaryFixingCheckBox->setVisible(toggled);
}

void QmitkRemeshingView::OnEdgeSplittingChanged(double edgeSplitting)
{
  if (edgeSplitting != m_Controls.edgeSplittingSlider->value())
    m_Controls.edgeSplittingSlider->setValue(edgeSplitting);

  if (edgeSplitting != m_Controls.edgeSplittingSpinBox->value())
    m_Controls.edgeSplittingSpinBox->setValue(edgeSplitting);
}

void QmitkRemeshingView::OnGradationChanged(double gradation)
{
  if (gradation != m_Controls.gradationSlider->value())
    m_Controls.gradationSlider->setValue(gradation);

  if (gradation != m_Controls.gradationSpinBox->value())
    m_Controls.gradationSpinBox->setValue(gradation);
}

void QmitkRemeshingView::OnMaxNumVerticesLineEditEditingFinished()
{
  int maximum = m_Controls.maxNumVerticesLineEdit->text().toInt();

  if (m_Controls.numVerticesSpinBox->maximum() != maximum)
  {
    m_Controls.numVerticesSlider->setMaximum(maximum);
    m_Controls.numVerticesSpinBox->setMaximum(maximum);
  }
}

void QmitkRemeshingView::OnNumberOfVerticesChanged(int numVertices)
{
  if (numVertices != m_Controls.numVerticesSlider->value())
    m_Controls.numVerticesSlider->setValue(numVertices);

  if (numVertices != m_Controls.numVerticesSpinBox->value())
    m_Controls.numVerticesSpinBox->setValue(numVertices);
}

void QmitkRemeshingView::OnOptimizationLevelChanged(int optimizationLevel)
{
  if (optimizationLevel != m_Controls.optimizationLevelSlider->value())
    m_Controls.optimizationLevelSlider->setValue(optimizationLevel);

  if (optimizationLevel != m_Controls.optimizationLevelSpinBox->value())
    m_Controls.optimizationLevelSpinBox->setValue(optimizationLevel);
}

void QmitkRemeshingView::OnRemeshButtonClicked()
{
  mitk::DataNode::Pointer selectedNode = m_Controls.surfaceComboBox->GetSelectedNode();
  mitk::Surface::ConstPointer surface = static_cast<mitk::Surface*>(selectedNode->GetData());
  int numVertices = m_Controls.numVerticesSpinBox->value();
  double gradation = m_Controls.gradationSpinBox->value();
  int subsampling = m_Controls.subsamplingSpinBox->value();
  double edgeSplitting = m_Controls.edgeSplittingSpinBox->value();
  int optimizationLevel = m_Controls.optimizationLevelSpinBox->value();
  bool forceManifold = m_Controls.forceManifoldCheckBox->isChecked();
  bool boundaryFixing = m_Controls.boundaryFixingCheckBox->isChecked();

  // mitk::Surface::Pointer remeshedSurface = mitk::ACVD::Remesh(surface, 0, numVertices, gradation, subsampling, edgeSplitting, optimizationLevel, forceManifold, boundaryFixing);

  mitk::ACVD::RemeshFilter::Pointer remesher = mitk::ACVD::RemeshFilter::New();
  remesher->SetInput(surface);
  remesher->SetTimeStep(0);
  remesher->SetNumVertices(numVertices);
  remesher->SetGradation(gradation);
  remesher->SetSubsampling(subsampling);
  remesher->SetEdgeSplitting(edgeSplitting);
  remesher->SetOptimizationLevel(optimizationLevel);
  remesher->SetForceManifold(forceManifold);
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
  newNode->SetName(QString("%1 (%2, %3)").arg(selectedNode->GetName().c_str()).arg(remeshedSurface->GetVtkPolyData()->GetNumberOfPoints()).arg(gradation).toStdString());
  newNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));
  newNode->SetData(remeshedSurface);

  this->GetDataStorage()->Add(newNode, selectedNode);
}

void QmitkRemeshingView::OnSelectedSurfaceChanged(const mitk::DataNode *node)
{
  if (node != NULL)
  {
    int numVertices = static_cast<int>(static_cast<mitk::Surface*>(node->GetData())->GetVtkPolyData()->GetNumberOfPoints());
    int minimum = numVertices < 100 ? numVertices : 100;
    int maximum = numVertices == minimum ? numVertices * 10 : numVertices;
    int step = std::max(1, maximum / 10);

    this->SetNumberOfVertices(minimum, maximum, step, numVertices);
    this->EnableWidgets(true);
  }
  else
  {
    this->EnableWidgets(false);
    this->SetNumberOfVertices(0, 0, 0, 0);
  }
}

void QmitkRemeshingView::OnSubsamplingChanged(int subsampling)
{
  if (subsampling != m_Controls.subsamplingSlider->value())
    m_Controls.subsamplingSlider->setValue(subsampling);

  if (subsampling != m_Controls.subsamplingSpinBox->value())
    m_Controls.subsamplingSpinBox->setValue(subsampling);
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

  m_Controls.maxNumVerticesLineEdit->setText(QString("%1").arg(maximum));
}
