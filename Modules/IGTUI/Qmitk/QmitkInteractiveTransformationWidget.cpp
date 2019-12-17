/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkInteractiveTransformationWidget.h"

// mitk includes
#include "mitkRenderingManager.h"
#include "mitkBaseRenderer.h"
#include "mitkNavigationData.h"

// vtk includes
#include "vtkMatrix4x4.h"
#include "vtkLinearTransform.h"

const std::string QmitkInteractiveTransformationWidget::VIEW_ID = "org.mitk.views.interactivetransformationwidget";

QmitkInteractiveTransformationWidget::QmitkInteractiveTransformationWidget(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f), m_Controls(nullptr), m_Geometry(nullptr)
{
  CreateQtPartControl(this);
  CreateConnections();

  m_ResetGeometry = mitk::Geometry3D::New();


  this->setWindowTitle("Edit Tool Tip and Tool Orientation");
}

QmitkInteractiveTransformationWidget::~QmitkInteractiveTransformationWidget()
{
}

void QmitkInteractiveTransformationWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkInteractiveTransformationWidgetControls;
    m_Controls->setupUi(parent);
  }
}

void QmitkInteractiveTransformationWidget::CreateConnections()
{
  if (m_Controls)
  {
    // translations
    connect(m_Controls->m_XTransSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QmitkInteractiveTransformationWidget::OnXTranslationValueChanged);
    connect(m_Controls->m_XTransSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnXTranslationValueChanged);

    connect(m_Controls->m_YTransSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QmitkInteractiveTransformationWidget::OnYTranslationValueChanged);
    connect(m_Controls->m_YTransSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnYTranslationValueChanged);

    connect(m_Controls->m_ZTransSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QmitkInteractiveTransformationWidget::OnZTranslationValueChanged);
    connect(m_Controls->m_ZTransSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnZTranslationValueChanged);

    // rotations
    connect(m_Controls->m_XRotSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QmitkInteractiveTransformationWidget::OnXRotationValueChanged);
    connect(m_Controls->m_XRotSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnXRotationValueChanged);

    connect(m_Controls->m_YRotSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QmitkInteractiveTransformationWidget::OnYRotationValueChanged);
    connect(m_Controls->m_YRotSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnYRotationValueChanged);

    connect(m_Controls->m_ZRotSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QmitkInteractiveTransformationWidget::OnZRotationValueChanged);
    connect(m_Controls->m_ZRotSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnZRotationValueChanged);

    connect((QObject*)(m_Controls->m_ResetPB), SIGNAL(clicked()), this, SLOT(OnResetGeometryToIdentity()));
    connect((QObject*)(m_Controls->m_RevertChanges), SIGNAL(clicked()), this, SLOT(OnRevertChanges()));
    connect((QObject*)(m_Controls->m_UseManipulatedToolTipPB), SIGNAL(clicked()), this, SLOT(OnApplyManipulatedToolTip()));
    connect((QObject*)(m_Controls->m_Cancel), SIGNAL(clicked()), this, SLOT(OnCancel()));
  }
}

void QmitkInteractiveTransformationWidget::SetToolToEdit(const mitk::NavigationTool::Pointer _tool)
{
  //If there is already a tool, remove it's node first.
  if (m_ToolToEdit)
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))->GetDataStorage()
    ->Remove(m_ToolToEdit->GetDataNode());

  m_ToolToEdit = _tool->Clone();
  mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))->GetDataStorage()
    ->Add(m_ToolToEdit->GetDataNode());
  m_ToolToEdit->GetDataNode()->SetName("Tool Tip to be edited");

  //change color to red
  m_ToolToEdit->GetDataNode()->SetProperty("color", mitk::ColorProperty::New(1, 0, 0));

  //use the set-function via vtk matrix, 'cause this guarantees a deep copy and not just sharing a pointer.
  m_Geometry = m_ToolToEdit->GetDataNode()->GetData()->GetGeometry();
  m_ResetGeometry->SetIndexToWorldTransformByVtkMatrix(m_Geometry->GetVtkMatrix()); //Remember the original values to be able to reset and abort everything
}

void QmitkInteractiveTransformationWidget::SetDefaultOffset(const mitk::Point3D _defaultValues)
{
  m_Geometry->SetOrigin(_defaultValues);
  m_ResetGeometry->SetOrigin(_defaultValues); //Remember the original values to be able to reset and abort everything
  SetValuesToGUI(m_Geometry->GetIndexToWorldTransform());
}

void QmitkInteractiveTransformationWidget::SetDefaultRotation(const mitk::Quaternion _defaultValues)
{
  // Conversion to navigation data / transform
  mitk::NavigationData::Pointer rotationTransform = mitk::NavigationData::New(m_Geometry->GetIndexToWorldTransform());
  rotationTransform->SetOrientation(_defaultValues);
  m_Geometry->SetIndexToWorldTransform(rotationTransform->GetAffineTransform3D());

  //For ResetGeometry, use the set-function via vtk matrix, 'cause this guarantees a deep copy and not just sharing a pointer.
  m_ResetGeometry->SetIndexToWorldTransformByVtkMatrix(m_Geometry->GetVtkMatrix()); //Remember the original values to be able to reset and abort everything
  SetValuesToGUI(m_Geometry->GetIndexToWorldTransform());
}

void QmitkInteractiveTransformationWidget::SetValuesToGUI(const mitk::AffineTransform3D::Pointer _defaultValues)
{

  //Set toolTip values in gui
  m_Controls->m_XTransSlider->setValue(_defaultValues->GetOffset()[0]);
  m_Controls->m_YTransSlider->setValue(_defaultValues->GetOffset()[1]);
  m_Controls->m_ZTransSlider->setValue(_defaultValues->GetOffset()[2]);

  //first: some conversion
  mitk::NavigationData::Pointer transformConversionHelper = mitk::NavigationData::New(_defaultValues);
  double eulerAlphaDegrees = transformConversionHelper->GetOrientation().rotation_euler_angles()[0] / vnl_math::pi * 180;
  double eulerBetaDegrees = transformConversionHelper->GetOrientation().rotation_euler_angles()[1] / vnl_math::pi * 180;
  double eulerGammaDegrees = transformConversionHelper->GetOrientation().rotation_euler_angles()[2] / vnl_math::pi * 180;

  m_Controls->m_XRotSpinBox->setValue(eulerAlphaDegrees);
  m_Controls->m_YRotSpinBox->setValue(eulerBetaDegrees);
  m_Controls->m_ZRotSpinBox->setValue(eulerGammaDegrees);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::SetSynchronizedValuesToSliderAndSpinbox(QDoubleSpinBox* _spinbox, QSlider* _slider, double _value)
{
//block signals to avoid loop between slider and spinbox. Unblock at the end of the function!
  _spinbox->blockSignals(true);
  _slider->blockSignals(true);
  _spinbox->setValue(_value);
  _slider->setValue(_value);
//unblock signals. See above, don't remove this line. Unblock at the end of the function!
  _spinbox->blockSignals(false);//
  _slider->blockSignals(false);//
}

void QmitkInteractiveTransformationWidget::OnXTranslationValueChanged(double v)
{
  //Set values to member variable
  mitk::Point3D translationParams = m_Geometry->GetOrigin();
  translationParams[0] = v;
  m_Geometry->SetOrigin(translationParams);

  SetSynchronizedValuesToSliderAndSpinbox(m_Controls->m_XTransSpinBox, m_Controls->m_XTransSlider, v);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkInteractiveTransformationWidget::OnYTranslationValueChanged(double v)
{
  //Set values to member variable
  mitk::Point3D translationParams = m_Geometry->GetOrigin();
  translationParams[1] = v;
  m_Geometry->SetOrigin(translationParams);

  SetSynchronizedValuesToSliderAndSpinbox(m_Controls->m_YTransSpinBox, m_Controls->m_YTransSlider, v);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnZTranslationValueChanged(double v)
{
  //Set values to member variable
  mitk::Point3D translationParams = m_Geometry->GetOrigin();
  translationParams[2] = v;
  m_Geometry->SetOrigin(translationParams);

  SetSynchronizedValuesToSliderAndSpinbox(m_Controls->m_ZTransSpinBox, m_Controls->m_ZTransSlider, v);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnXRotationValueChanged(double v)
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = v;
  rotationParams[1] = m_Controls->m_YRotSpinBox->value();
  rotationParams[2] = m_Controls->m_ZRotSpinBox->value();

  SetSynchronizedValuesToSliderAndSpinbox(m_Controls->m_XRotSpinBox, m_Controls->m_XRotSlider, v);

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::OnYRotationValueChanged(double v)
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = m_Controls->m_XRotSpinBox->value();
  rotationParams[1] = v;
  rotationParams[2] = m_Controls->m_ZRotSpinBox->value();

  SetSynchronizedValuesToSliderAndSpinbox(m_Controls->m_YRotSpinBox, m_Controls->m_YRotSlider, v);

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::OnZRotationValueChanged(double v)
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = m_Controls->m_XRotSpinBox->value();
  rotationParams[1] = m_Controls->m_YRotSpinBox->value();
  rotationParams[2] = v;

  SetSynchronizedValuesToSliderAndSpinbox(m_Controls->m_ZRotSpinBox, m_Controls->m_ZRotSlider, v);

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::Rotate(mitk::Vector3D rotateVector)
{
  //0: from degrees to radians
  double radianX = rotateVector[0] * vnl_math::pi / 180;
  double radianY = rotateVector[1] * vnl_math::pi / 180;
  double radianZ = rotateVector[2] * vnl_math::pi / 180;

  //1: from euler angles to quaternion
  mitk::Quaternion rotation(radianX, radianY, radianZ);

  //2: Conversion to navigation data / transform
  mitk::NavigationData::Pointer rotationTransform = mitk::NavigationData::New(m_Geometry->GetIndexToWorldTransform());
  rotationTransform->SetOrientation(rotation);

  m_Geometry->SetIndexToWorldTransform(rotationTransform->GetAffineTransform3D());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnResetGeometryToIdentity()
{
  // reset the input to its initial state.
  m_Geometry->SetIdentity();

  //Update Sliders
  this->SetValuesToGUI(m_Geometry->GetIndexToWorldTransform());
  //Refresh view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnRevertChanges()
{
  // reset the input to its initial state.
  m_Geometry->SetIndexToWorldTransformByVtkMatrix(m_ResetGeometry->GetVtkMatrix());

  //Update Sliders
  this->SetValuesToGUI(m_Geometry->GetIndexToWorldTransform());
  //Refresh view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnApplyManipulatedToolTip()
{
  mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))->GetDataStorage()
    ->Remove(m_ToolToEdit->GetDataNode());

  mitk::AffineTransform3D::Pointer toolTip = m_Geometry->GetIndexToWorldTransform();
  emit EditToolTipFinished(toolTip);
  this->close();
}

void QmitkInteractiveTransformationWidget::reject()
{
  OnCancel();
}

void QmitkInteractiveTransformationWidget::OnCancel()
{
  QDialog::reject();

  mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))->GetDataStorage()
    ->Remove(m_ToolToEdit->GetDataNode());

  emit EditToolTipFinished(nullptr);
}
