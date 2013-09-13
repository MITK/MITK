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
: QWidget(parent, f), m_Geometry(NULL), m_ResetGeometry(NULL), m_Controls(NULL)
{
  CreateQtPartControl(this);
  CreateConnections();
  m_TranslationVector.Fill(0.0f);
  m_RotateSliderPos.Fill(0.0f);
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
  if ( m_Controls )
  {
    // translations
    connect( (QObject*)(m_Controls->m_XTransSlider), SIGNAL(valueChanged(int)), this, SLOT(OnXTranslationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_XTransSpinBox), SIGNAL(valueChanged(int)), this, SLOT(OnXTranslationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_YTransSlider), SIGNAL(valueChanged(int)), this, SLOT(OnYTranslationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_YTransSpinBox), SIGNAL(valueChanged(int)), this, SLOT(OnYTranslationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_ZTransSlider), SIGNAL(valueChanged(int)), this, SLOT(OnZTranslationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_ZTransSpinBox), SIGNAL(valueChanged(int)), this, SLOT(OnZTranslationValueChanged(int)) );

    // rotations
    connect( (QObject*)(m_Controls->m_XRotSlider), SIGNAL(valueChanged(int)), this, SLOT(OnXRotationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_XRotSpinBox), SIGNAL(valueChanged(int)), this, SLOT(OnXRotationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_YRotSlider), SIGNAL(valueChanged(int)), this, SLOT(OnYRotationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_YRotSpinBox), SIGNAL(valueChanged(int)), this, SLOT(OnYRotationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_ZRotSlider), SIGNAL(valueChanged(int)), this, SLOT(OnZRotationValueChanged(int)) );
    connect( (QObject*)(m_Controls->m_ZRotSpinBox), SIGNAL(valueChanged(int)), this, SLOT(OnZRotationValueChanged(int)) );

    connect( (QObject*)(m_Controls->m_ResetPB), SIGNAL(clicked()), this, SLOT(OnResetGeometry()) );
    connect( (QObject*)(m_Controls->m_UseManipulatedToolTipPB), SIGNAL(clicked()), this, SLOT(OnApplyManipulatedToolTip()) );
  }
}

void QmitkInteractiveTransformationWidget::SetGeometry( mitk::Geometry3D::Pointer geometry, mitk::Geometry3D::Pointer defaultValues )
{
  m_Geometry = geometry;
  m_ResetGeometry = geometry->Clone();

  //set default values
  if (defaultValues.IsNotNull())
  {
  //first: some conversion
  mitk::NavigationData::Pointer transformConversionHelper = mitk::NavigationData::New(defaultValues->GetIndexToWorldTransform());
  double eulerAlphaDegrees = transformConversionHelper->GetOrientation().rotation_euler_angles()[0] / vnl_math::pi * 180;
  double eulerBetaDegrees = transformConversionHelper->GetOrientation().rotation_euler_angles()[1] / vnl_math::pi * 180;
  double eulerGammaDegrees = transformConversionHelper->GetOrientation().rotation_euler_angles()[2] / vnl_math::pi * 180;

  //set translation
  OnXTranslationValueChanged(defaultValues->GetIndexToWorldTransform()->GetOffset()[0]);
  OnYTranslationValueChanged(defaultValues->GetIndexToWorldTransform()->GetOffset()[1]);
  OnZTranslationValueChanged(defaultValues->GetIndexToWorldTransform()->GetOffset()[2]);

  //set rotation
  OnXRotationValueChanged(eulerAlphaDegrees);
  OnYRotationValueChanged(eulerBetaDegrees);
  OnZRotationValueChanged(eulerGammaDegrees);
  }
}

mitk::Geometry3D::Pointer QmitkInteractiveTransformationWidget::GetGeometry()
{
  return m_Geometry;
}





/////////////////////////////////////////////////////////////////////////////////////////////
// Section to allow interactive positioning of the moving surface
/////////////////////////////////////////////////////////////////////////////////////////////

void QmitkInteractiveTransformationWidget::OnXTranslationValueChanged( int v )
{
  mitk::Vector3D translationParams;
  translationParams[0] = v;
  translationParams[1] = m_Controls->m_YTransSlider->value();
  translationParams[2] = m_Controls->m_ZTransSlider->value();
  SetSliderX(v);
  this->Translate(translationParams);
}

void QmitkInteractiveTransformationWidget::SetSliderX(int v)
{
  m_Controls->m_XTransSlider->setValue(v);
  m_Controls->m_XTransSpinBox->setValue(v);
}

void QmitkInteractiveTransformationWidget::OnYTranslationValueChanged( int v )
{
  mitk::Vector3D translationParams;
  translationParams[0] = m_Controls->m_XTransSlider->value();
  translationParams[1] = v;
  translationParams[2] = m_Controls->m_ZTransSlider->value();
  SetSliderY(v);
  this->Translate(translationParams);
}

void QmitkInteractiveTransformationWidget::SetSliderY(int v)
{
  m_Controls->m_YTransSlider->setValue(v);
  m_Controls->m_YTransSpinBox->setValue(v);
}

void QmitkInteractiveTransformationWidget::OnZTranslationValueChanged( int v )
{
  mitk::Vector3D translationParams;
  translationParams[0] = m_Controls->m_XTransSlider->value();
  translationParams[1] = m_Controls->m_YTransSlider->value();
  translationParams[2] = v;
  SetSliderZ(v);
  this->Translate(translationParams);
}

void QmitkInteractiveTransformationWidget::SetSliderZ(int v)
{
  m_Controls->m_ZTransSlider->setValue(v);
  m_Controls->m_ZTransSpinBox->setValue(v);
}

void QmitkInteractiveTransformationWidget::Translate( mitk::Vector3D translateVector)
{
  mitk::Vector3D translateVec;

  // transform the translation vector
  translateVec[0] = translateVector[0] - m_TranslationVector[0];
  translateVec[1] = translateVector[1] - m_TranslationVector[1];
  translateVec[2] = translateVector[2] - m_TranslationVector[2];

  // set the new translation vector to member variable
  m_TranslationVector[0] = translateVector[0];
  m_TranslationVector[1] = translateVector[1];
  m_TranslationVector[2] = translateVector[2];

  m_Geometry->Translate( translateVec );
  qApp->processEvents();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnXRotationValueChanged( int v )
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = v;
  rotationParams[1] = m_Controls->m_YRotSlider->value();
  rotationParams[2] = m_Controls->m_ZRotSlider->value();

  m_Controls->m_XRotSlider->setValue(v);
  m_Controls->m_XRotSpinBox->setValue(v);

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::OnYRotationValueChanged( int v )
{
  mitk::Vector3D rotationParams;
  rotationParams[0] = m_Controls->m_XRotSlider->value();
  rotationParams[1] = v;
  rotationParams[2] = m_Controls->m_ZRotSlider->value();

  m_Controls->m_YRotSlider->setValue(v);
  m_Controls->m_YRotSpinBox->setValue(v);

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::OnZRotationValueChanged( int v )
{
  mitk::Vector3D rotationParams;
  rotationParams[0]=m_Controls->m_XRotSlider->value();
  rotationParams[1]=m_Controls->m_YRotSlider->value();
  rotationParams[2]=v;
  m_Controls->m_ZRotSlider->setValue(v);
  m_Controls->m_ZRotSpinBox->setValue(v);

  this->Rotate(rotationParams);
}

void QmitkInteractiveTransformationWidget::Rotate(mitk::Vector3D rotateVector)
{
  //0: from degrees to radians
  double radianX = rotateVector[0] * vnl_math::pi / 180;
  double radianY = rotateVector[1] * vnl_math::pi / 180;
  double radianZ = rotateVector[2] * vnl_math::pi / 180;
  //1: from euler angles to quaternion
  mitk::Quaternion rotation(radianX,radianY,radianZ) ;
  //2: Conversion to navigation data / transform
  mitk::NavigationData::Pointer rotationTransform = mitk::NavigationData::New();
  rotationTransform->SetOrientation(rotation);
  //3: Apply transform
  m_Geometry->SetIndexToWorldTransform(rotationTransform->GetAffineTransform3D());
  qApp->processEvents();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnResetGeometry()
{
  m_Controls->m_XRotSlider->setValue(0);
  m_Controls->m_YRotSlider->setValue(0);
  m_Controls->m_ZRotSlider->setValue(0);
  m_Controls->m_XRotSpinBox->setValue(0);
  m_Controls->m_YRotSpinBox->setValue(0);
  m_Controls->m_ZRotSpinBox->setValue(0);

  m_Controls->m_XTransSlider->setValue(0);
  m_Controls->m_YTransSlider->setValue(0);
  m_Controls->m_ZTransSlider->setValue(0);
  m_Controls->m_XTransSpinBox->setValue(0);
  m_Controls->m_YTransSpinBox->setValue(0);
  m_Controls->m_ZTransSpinBox->setValue(0);
  qApp->processEvents();

  // reset the input to its initial state.
  m_ResetGeometry->TransferItkToVtkTransform();
  m_Geometry->SetIdentity();
  m_Geometry->Compose(m_ResetGeometry->GetVtkTransform()->GetMatrix());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkInteractiveTransformationWidget::OnApplyManipulatedToolTip()
{
  emit ApplyManipulatedToolTip();
}

itk::Matrix<double,3,3> QmitkInteractiveTransformationWidget::ConvertEulerAnglesToRotationMatrix(double alpha, double beta, double gamma)
{
    double PI = vnl_math::pi;
    alpha = alpha * PI / 180;
    beta = beta * PI / 180;
    gamma = gamma * PI / 180;

    //convert angles to matrix:
    itk::Matrix<double,3,3> matrix;

    //Luftfahrtnorm (DIN 9300) (Yaw-Pitch-Roll, Z, Y, X)
    matrix[0][0] = cos(beta) * cos(alpha);
    matrix[0][1] = cos(beta) * sin(alpha);
    matrix[0][2] = -sin(beta);

    matrix[1][0] = sin(gamma) * sin(beta) * cos(alpha) - cos(gamma) * sin(alpha) ;
    matrix[1][1] = sin(gamma) * sin(beta) * sin(alpha) + cos(gamma) * cos(alpha);
    matrix[1][2] = sin(gamma) * cos(beta);

    matrix[2][0] = cos(gamma) * sin(beta) * cos(alpha) + sin(gamma) * sin(alpha);
    matrix[2][1] = cos(gamma) * sin(beta) * sin(alpha) - sin(gamma) * cos(alpha);
    matrix[2][2] = cos(gamma) * cos(beta);

    return matrix;
}