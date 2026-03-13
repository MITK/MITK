/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//Qmitk
#include "QmitkRegistrationManipulationWidget.h"

//MatchPoint
#include <mapRegistrationManipulator.h>
#include <mapPreCachedRegistrationKernel.h>
#include <mapCombinedRegistrationKernel.h>
#include <mapNullRegistrationKernel.h>
#include <mapRegistrationCombinator.h>

#include <itkCompositeTransform.h>

#include <boost/math/constants/constants.hpp>

#include <ui_QmitkRegistrationManipulationWidget.h>

QmitkRegistrationManipulationWidget::QmitkRegistrationManipulationWidget(QWidget *parent)
  : QWidget(parent), m_Controls(std::make_unique<Ui::QmitkRegistrationManipulationWidget>()), m_CenterOfRotationIsRelativeToTarget(false), m_internalUpdate(false)
{
  m_Controls->setupUi(this);

  connect(m_Controls->slideRotX, SIGNAL(valueChanged(int)), this, SLOT(OnRotXSlideChanged(int)));
  connect(m_Controls->sbRotX, SIGNAL(valueChanged(double)), this, SLOT(OnRotXChanged(double)));
  connect(m_Controls->slideRotY, SIGNAL(valueChanged(int)), this, SLOT(OnRotYSlideChanged(int)));
  connect(m_Controls->sbRotY, SIGNAL(valueChanged(double)), this, SLOT(OnRotYChanged(double)));
  connect(m_Controls->slideRotZ, SIGNAL(valueChanged(int)), this, SLOT(OnRotZSlideChanged(int)));
  connect(m_Controls->sbRotZ, SIGNAL(valueChanged(double)), this, SLOT(OnRotZChanged(double)));

  connect(m_Controls->slideTransX, SIGNAL(valueChanged(int)), this, SLOT(OnTransXSlideChanged(int)));
  connect(m_Controls->sbTransX, SIGNAL(valueChanged(double)), this, SLOT(OnTransXChanged(double)));
  connect(m_Controls->slideTransY, SIGNAL(valueChanged(int)), this, SLOT(OnTransYSlideChanged(int)));
  connect(m_Controls->sbTransY, SIGNAL(valueChanged(double)), this, SLOT(OnTransYChanged(double)));
  connect(m_Controls->slideTransZ, SIGNAL(valueChanged(int)), this, SLOT(OnTransZSlideChanged(int)));
  connect(m_Controls->sbTransZ, SIGNAL(valueChanged(double)), this, SLOT(OnTransZChanged(double)));

  m_Controls->groupScale->setVisible(false);
}

QmitkRegistrationManipulationWidget::~QmitkRegistrationManipulationWidget()
{
}

void QmitkRegistrationManipulationWidget::Initialize()
{
  this->ResetTransforms();
  this->InitControls();
};

void QmitkRegistrationManipulationWidget::Initialize(MAPRegistrationType* precedingRegistration)
{
  this->ResetTransforms();
  this->m_PreRegistration = precedingRegistration;

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(m_CurrentRegistration);
  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();

  const map::core::RegistrationKernel<3, 3>* preKernel = dynamic_cast<const map::core::RegistrationKernel<3, 3>*>(&this->m_PreRegistration->getInverseMapping());
  itk::CompositeTransform < ::map::core::continuous::ScalarType, 3>::Pointer compTransform = itk::CompositeTransform < ::map::core::continuous::ScalarType, 3>::New();
  compTransform->AddTransform(preKernel->getTransformModel()->Clone());
  compTransform->AddTransform(this->m_InverseCurrentTransform);

  kernel->setTransformModel(compTransform);
  manipulator.setInverseMapping(kernel);

  this->InitControls();
};

void QmitkRegistrationManipulationWidget::Initialize(const mitk::Point3D& movingReference, const mitk::Point3D& targetReference)
{
  this->ResetTransforms();

  auto offset = targetReference - movingReference;
  m_DirectCurrentTransform->SetOffset(offset);
  m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);

  this->InitControls();
};

void QmitkRegistrationManipulationWidget::ResetTransforms()
{
  this->m_CenterOfRotation.Fill(0.0);
  this->m_PreRegistration = nullptr;


  this->m_InverseCurrentTransform = TransformType::New();
  this->m_InverseCurrentTransform->SetIdentity();
  this->m_DirectCurrentTransform = TransformType::New();
  this->m_DirectCurrentTransform->SetIdentity();

  m_CurrentRegistration = MAPRegistrationType::New();

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(m_CurrentRegistration);
  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel->setTransformModel(m_InverseCurrentTransform);
  manipulator.setInverseMapping(kernel);

  manipulator.setDirectMapping(::map::core::NullRegistrationKernel < 3, 3 >::New());
};

void QmitkRegistrationManipulationWidget::SetCenterOfRotation(const mitk::Point3D& center)
{
  this->m_CenterOfRotation = center;
  this->ConfigureTransformCenter();
  this->UpdateTransformWidgets();
};

/** Sets the internal m_CenterOfRotationIsRelativeToTarget. see below.*/
void QmitkRegistrationManipulationWidget::SetCenterOfRotationIsRelativeToTarget(bool targetRelative)
{
  this->m_CenterOfRotationIsRelativeToTarget = targetRelative;
  this->ConfigureTransformCenter();
  this->UpdateTransformWidgets();
};

void QmitkRegistrationManipulationWidget::InitControls()
{
  this->ConfigureTransformCenter();

  //set bounds of the translation slider widget to have sensible ranges
  this->m_internalUpdate = true;
  auto currenttrans = m_DirectCurrentTransform->GetTranslation();
  m_Controls->slideTransX->setMinimum(currenttrans[0] - 250);
  m_Controls->slideTransY->setMinimum(currenttrans[1] - 250);
  m_Controls->slideTransZ->setMinimum(currenttrans[2] - 250);
  m_Controls->slideTransX->setMaximum(currenttrans[0] + 250);
  m_Controls->slideTransY->setMaximum(currenttrans[1] + 250);
  m_Controls->slideTransZ->setMaximum(currenttrans[2] + 250);
  this->m_internalUpdate = false;

  this->UpdateTransformWidgets();
};

void QmitkRegistrationManipulationWidget::UpdateTransformWidgets()
{
  this->m_internalUpdate = true;
  m_Controls->sbTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
  m_Controls->sbTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
  m_Controls->sbTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);
  m_Controls->slideTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
  m_Controls->slideTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
  m_Controls->slideTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);

  m_Controls->sbRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
  m_Controls->sbRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
  m_Controls->sbRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
  m_Controls->slideRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
  m_Controls->slideRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
  m_Controls->slideRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
  this->m_internalUpdate = false;
};

void QmitkRegistrationManipulationWidget::UpdateTransform(bool updateRotation)
{
  if (updateRotation)
  {
    if (this->m_CenterOfRotationIsRelativeToTarget)
    {
     ConfigureTransformCenter();
    }

    this->m_DirectCurrentTransform->SetRotation(m_Controls->sbRotX->value()*(boost::math::double_constants::pi / 180),
      m_Controls->sbRotY->value()*(boost::math::double_constants::pi / 180),
      m_Controls->sbRotZ->value()*(boost::math::double_constants::pi / 180));
  }
  else
  {
    TransformType::OutputVectorType trans;
    trans[0] = m_Controls->sbTransX->value();
    trans[1] = m_Controls->sbTransY->value();
    trans[2] = m_Controls->sbTransZ->value();

    this->m_DirectCurrentTransform->SetTranslation(trans);
  }

  this->m_DirectCurrentTransform->GetInverse(this->m_InverseCurrentTransform);

  this->UpdateTransformWidgets();

  emit RegistrationChanged(this->m_CurrentRegistration);
};

map::core::RegistrationBase* QmitkRegistrationManipulationWidget::GetInterimRegistration() const
{
  return this->m_CurrentRegistration.GetPointer();
};

map::core::RegistrationBase::Pointer QmitkRegistrationManipulationWidget::GenerateRegistration() const
{
  MAPRegistrationType::Pointer newReg = MAPRegistrationType::New();

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(newReg);

  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel->setTransformModel(m_InverseCurrentTransform);

  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel2 = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel2->setTransformModel(m_InverseCurrentTransform->GetInverseTransform());

  manipulator.setInverseMapping(kernel);
  manipulator.setDirectMapping(kernel2);

  if (this->m_PreRegistration.IsNotNull())
  { //compine registration with selected pre registration as baseline
    typedef ::map::core::RegistrationCombinator<MAPRegistrationType, MAPRegistrationType> CombinatorType;
    CombinatorType::Pointer combinator = CombinatorType::New();
    newReg = combinator->process(*m_PreRegistration, *newReg);
  }

  return newReg.GetPointer();
};

void QmitkRegistrationManipulationWidget::OnRotXChanged(double x)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    m_Controls->slideRotX->setValue(x);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkRegistrationManipulationWidget::OnRotXSlideChanged(int x)
{
  if (!m_internalUpdate)
  {
    m_Controls->sbRotX->setValue(x);
  }
};

void QmitkRegistrationManipulationWidget::OnRotYChanged(double y)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    m_Controls->slideRotY->setValue(y);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkRegistrationManipulationWidget::OnRotYSlideChanged(int y)
{
  if (!m_internalUpdate)
  {
    m_Controls->sbRotY->setValue(y);
  }
};

void QmitkRegistrationManipulationWidget::OnRotZChanged(double z)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    m_Controls->slideRotZ->setValue(z);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkRegistrationManipulationWidget::OnRotZSlideChanged(int z)
{
  if (!m_internalUpdate)
  {
    m_Controls->sbRotZ->setValue(z);
  }
};

void QmitkRegistrationManipulationWidget::OnTransXChanged(double x)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    m_Controls->slideTransX->setValue(x);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkRegistrationManipulationWidget::OnTransXSlideChanged(int x)
{
  if (!m_internalUpdate)
  {
    m_Controls->sbTransX->setValue(x);
  }
};

void QmitkRegistrationManipulationWidget::OnTransYChanged(double y)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    m_Controls->slideTransY->setValue(y);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkRegistrationManipulationWidget::OnTransYSlideChanged(int y)
{
  if (!m_internalUpdate)
  {
    m_Controls->sbTransY->setValue(y);
  }
};

void QmitkRegistrationManipulationWidget::OnTransZChanged(double z)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    m_Controls->slideTransZ->setValue(z);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkRegistrationManipulationWidget::OnTransZSlideChanged(int z)
{
  if (!m_internalUpdate)
  {
    m_Controls->sbTransZ->setValue(z);
  }
};

void QmitkRegistrationManipulationWidget::ConfigureTransformCenter()
{
  auto offset = m_DirectCurrentTransform->GetOffset();


  if (this->m_CenterOfRotationIsRelativeToTarget)
  {
    auto newCenter = m_InverseCurrentTransform->TransformPoint(m_CenterOfRotation);
    m_DirectCurrentTransform->SetCenter(newCenter);
  }
  else
  {
    m_DirectCurrentTransform->SetCenter(m_CenterOfRotation);
  }

  m_DirectCurrentTransform->SetOffset(offset);
  m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);
};
