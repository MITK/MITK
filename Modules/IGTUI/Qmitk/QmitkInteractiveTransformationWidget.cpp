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

#include "QmitkInteractiveTransformationWidget.h"

// mitk includes
#include "mitkRenderingManager.h"
#include "mitkNavigationData.h"

// vtk includes
#include "vtkMatrix4x4.h"
#include "vtkLinearTransform.h"

const std::string QmitkInteractiveTransformationWidget::VIEW_ID = "org.mitk.views.interactivetransformationwidget";

QmitkInteractiveTransformationWidget::QmitkInteractiveTransformationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_Controls(nullptr), m_Geometry(nullptr), m_ResetGeometry(nullptr)
{
  CreateQtPartControl(this);
  CreateConnections();
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
    connect(m_Controls->m_XTransSlider, &QSlider::valueChanged, m_Controls->m_XTransSpinBox, &QSpinBox::setValue);
    connect(m_Controls->m_XTransSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_Controls->m_XTransSlider, &QSlider::setValue);
    connect(m_Controls->m_XTransSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnXTranslationValueChanged);

    connect(m_Controls->m_YTransSlider, &QSlider::valueChanged, m_Controls->m_YTransSpinBox, &QSpinBox::setValue);
    connect(m_Controls->m_YTransSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_Controls->m_YTransSlider, &QSlider::setValue);
    connect(m_Controls->m_YTransSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnYTranslationValueChanged);

    connect(m_Controls->m_ZTransSlider, &QSlider::valueChanged, m_Controls->m_ZTransSpinBox, &QSpinBox::setValue);
    connect(m_Controls->m_ZTransSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_Controls->m_ZTransSlider, &QSlider::setValue);
    connect(m_Controls->m_ZTransSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnZTranslationValueChanged);

    // rotations
    connect(m_Controls->m_XRotSlider, &QSlider::valueChanged, m_Controls->m_XRotSpinBox, &QSpinBox::setValue);
    connect(m_Controls->m_XRotSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_Controls->m_XRotSlider, &QSlider::setValue);
    connect(m_Controls->m_XRotSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnXRotationValueChanged);

    connect(m_Controls->m_YRotSlider, &QSlider::valueChanged, m_Controls->m_YRotSpinBox, &QSpinBox::setValue);
    connect(m_Controls->m_YRotSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_Controls->m_YRotSlider, &QSlider::setValue);
    connect(m_Controls->m_YRotSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnYRotationValueChanged);

    connect(m_Controls->m_ZRotSlider, &QSlider::valueChanged, m_Controls->m_ZRotSpinBox, &QSpinBox::setValue);
    connect(m_Controls->m_ZRotSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_Controls->m_ZRotSlider, &QSlider::setValue);
    connect(m_Controls->m_ZRotSlider, &QSlider::valueChanged, this, &QmitkInteractiveTransformationWidget::OnZRotationValueChanged);

    connect((QObject*)(m_Controls->m_ResetPB), SIGNAL(clicked()), this, SLOT(OnResetGeometryToIdentity()));
    connect((QObject*)(m_Controls->m_RevertChanges), SIGNAL(clicked()), this, SLOT(OnRevertChanges()));
    connect((QObject*)(m_Controls->m_UseManipulatedToolTipPB), SIGNAL(clicked()), this, SLOT(OnApplyManipulatedToolTip()));
  }
}

void QmitkInteractiveTransformationWidget::SetGeometryPointer(mitk::BaseGeometry::Pointer geometry)
{
  m_Geometry = geometry;
  m_ResetGeometry = geometry->Clone(); //Remember the original values to be able to reset and abort everything
}

void QmitkInteractiveTransformationWidget::SetValues(const mitk::AffineTransform3D::Pointer _defaultValues)
{
  m_Geometry->SetIndexToWorldTransform(_defaultValues);

  //Set toolTip values in gui, slider will be adapted from spinbox via signal -> valueChanged
  m_Controls->m_XTransSpinBox->setValue(_defaultValues->GetOffset()[0]);
  m_Controls->m_YTransSpinBox->setValue(_defaultValues->GetOffset()[1]);
  m_Controls->m_ZTransSpinBox->setValue(_defaultValues->GetOffset()[2]);

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

/////////////////////////////////////////////////////////////////////////////////////////////
// Section to allow interactive positioning of the moving surface
/////////////////////////////////////////////////////////////////////////////////////////////

void QmitkInteractiveTransformationWidget::OnXTranslationValueChanged(int v)
{
  mitk::Vector3D translationParams;
  translationParams[0] = v;
  translationParams[1] = m_Controls->m_YTransSlider->value();
  translationParams[2] = m_Controls->m_ZTransSlider->value();
  m_Geometry->SetOrigin(translationParams);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnYTranslationValueChanged(int v)
{
  mitk::Vector3D translationParams;
  translationParams[0] = m_Controls->m_XTransSlider->value();
  translationParams[1] = v;
  translationParams[2] = m_Controls->m_ZTransSlider->value();
  m_Geometry->SetOrigin(translationParams);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnZTranslationValueChanged(int v)
{
  mitk::Vector3D translationParams;
  translationParams[0] = m_Controls->m_XTransSlider->value();
  translationParams[1] = m_Controls->m_YTransSlider->value();
  translationParams[2] = v;
  m_Geometry->SetOrigin(translationParams);

  //Update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnXRotationValueChanged(int v)
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = v;
  rotationParams[1] = m_Controls->m_YRotSlider->value();
  rotationParams[2] = m_Controls->m_ZRotSlider->value();

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::OnYRotationValueChanged(int v)
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = m_Controls->m_XRotSlider->value();
  rotationParams[1] = v;
  rotationParams[2] = m_Controls->m_ZRotSlider->value();

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::OnZRotationValueChanged(int v)
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = m_Controls->m_XRotSlider->value();
  rotationParams[1] = m_Controls->m_YRotSlider->value();
  rotationParams[2] = v;

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

  //Todo Slider anpassen
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnRevertChanges()
{
  // reset the input to its initial state.
  m_Geometry->SetIndexToWorldTransform(m_ResetGeometry->GetIndexToWorldTransform());

  //Todo Slider anpassen
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnApplyManipulatedToolTip()
{
  mitk::AffineTransform3D::Pointer toolTip = m_Geometry->GetIndexToWorldTransform();
  emit EditToolTipFinished(toolTip);
}